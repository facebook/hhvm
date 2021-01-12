#include <sqlite3.h>

#include <boost/program_options.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/numeric/conversion/cast.hpp>
#include <boost/filesystem.hpp>
#include <boost/progress.hpp>
#include <boost/lexical_cast.hpp>

#include <iostream>
#include <iomanip>
#include <stdexcept>
#include <limits>
#include <cstdint>
#include <cmath>
#include <set>
#include <string>

namespace po = boost::program_options;

typedef boost::shared_ptr<sqlite3> dbconn_ptr_t;
typedef boost::shared_ptr<sqlite3_stmt> stmt_ptr_t;

typedef std::pair<std::uint32_t, std::uint32_t> edge_t;
typedef std::vector<edge_t> edges_t;

// Open a DB handle.
dbconn_ptr_t open_db(std::string const& conn) {
  sqlite3* db = nullptr;
  int err = sqlite3_open_v2(conn.c_str(), &db, SQLITE_OPEN_READONLY, NULL);
  if (db == nullptr || err != SQLITE_OK) {
    throw(std::runtime_error(
      "Error: Couldn't open \"" + conn +
      "\". Error code " + boost::lexical_cast<std::string>(err)));
  }

  return dbconn_ptr_t(db, sqlite3_close_v2);
}

// Compile an SQL query.
stmt_ptr_t prepare_stmt(dbconn_ptr_t db, std::string const& sql) {
  sqlite3_stmt* stmt = nullptr;
  int err = sqlite3_prepare_v2(db.get(), sql.c_str(), -1, &stmt, NULL);
  if (stmt == nullptr || err != SQLITE_OK) {
    throw(std::runtime_error(
      "Error: Couldn't prepare statment \"" + sql +
      "\". Error code " + boost::lexical_cast<std::string>(err)));
  }

  return stmt_ptr_t(stmt, sqlite3_finalize);
}

// Retrieve the 'key_vertex' column of a row.
std::uint32_t key_vertex(stmt_ptr_t stmt) {
  return boost::numeric_cast<std::uint32_t>(sqlite3_column_int(stmt.get(), 0));
}

// Retrieve the 'value_vertex' column of a row.
std::set<std::uint32_t> value_vertex(stmt_ptr_t stmt) {
  int num_bytes = sqlite3_column_bytes(stmt.get(), 1);
  int n = sizeof(std::uint32_t); // 4 bytes
  if (num_bytes % n) {
    throw std::runtime_error("Error: Byte alignment mismatch");
  }
  int num_values = num_bytes / n;
  uint32_t const* values = reinterpret_cast<std::uint32_t const*>(sqlite3_column_blob(stmt.get(), 1));
  std::set<std::uint32_t> result(values, values + num_values);

  return result;
}

// Count the number of src nodes in a depgraph.
std::size_t count_key_vertex(dbconn_ptr_t db) {
  stmt_ptr_t count_stmt = prepare_stmt(db, "select count (*) from deptable;");
  sqlite3_step(count_stmt.get());
  return (std::size_t)(sqlite3_column_int(count_stmt.get(), 0));
}

// Print an ASCII representation of a depgraph to stdout.
void dump_depgraph(std::string const& file) {
  dbconn_ptr_t db = open_db(file);
  stmt_ptr_t stmt = prepare_stmt(db, "select * from deptable");
  int digits = std::log10(std::numeric_limits<std::uint32_t>::max()) + 1;
  while(sqlite3_step(stmt.get()) != SQLITE_DONE) {
    std::uint32_t src = key_vertex(stmt);
    std::set<std::uint32_t> dests = value_vertex(stmt);
    for(auto dst : dests) {
      std::cout << "  " << std::right << std::setw(digits) << src << " " << dst << '\n';
    }
  }
}

// Add edges to 'es' given source vertex 'k' and dest vertices 'vs'.
void add_edges(edges_t& es, std::uint32_t k, std::set<std::uint32_t> const& vs) {
  std::transform(vs.begin()
                 , vs.end()
                 , std::back_inserter(es)
                 , [=](auto const& v) { return std::make_pair (k, v); }
                 );
}

// Calculate the edges in 'r' not in 'l' (missing edges) and the edges
// in 'l' not in 'r' (extraneous edges).
void comp_depgraph(std::string const& l, std::string const& r) {
  dbconn_ptr_t ldb = open_db(l);
  dbconn_ptr_t rdb = open_db(r);

  stmt_ptr_t lstmt = prepare_stmt(ldb, "select * from deptable;");
  stmt_ptr_t rstmt = prepare_stmt(rdb, "select * from deptable;");

  std::size_t m = count_key_vertex(ldb);
  std::size_t n = count_key_vertex(rdb);

  edges_t missing, extra;
  unsigned int ledge_count = 0, redge_count = 0;
  boost::progress_display show_progress(std::max(m, n));
  int lc = sqlite3_step(lstmt.get()), rc = sqlite3_step(rstmt.get());
  while (!(lc == SQLITE_DONE && rc == SQLITE_DONE)) {
    if (lc == SQLITE_DONE) {
      // These edges are in 'r' and not in 'l'.
      auto k = key_vertex(rstmt);
      auto vs = value_vertex(rstmt);
      redge_count += vs.size();
      add_edges(missing, k, vs);
      rc = sqlite3_step(rstmt.get());
      if (n > m)
        ++show_progress; // We advanced 'r' and there are more keys in 'r' than 'l'.
      continue;
    }
    if (rc == SQLITE_DONE) {
      // These edges are in 'l' and not in 'r'.
      auto k = key_vertex(lstmt);
      auto vs = value_vertex(lstmt);
      ledge_count += vs.size();
      add_edges(extra, k, vs);
      lc = sqlite3_step(lstmt.get());
      if (n <= m)
        ++show_progress; // We advanced 'l' and there are more keys in 'l' than 'r'.
      continue;
    }

    auto lk = key_vertex(lstmt), rk = key_vertex(rstmt);
    auto lvs = value_vertex(lstmt), rvs = value_vertex(rstmt);
    if (lk < rk) {
      // These edges are in 'l' but not in 'r'.
      ledge_count += lvs.size();
      add_edges(extra, lk, lvs);
      lc = sqlite3_step(lstmt.get());
      if (n <= m)
        ++show_progress; // We advanced 'l' and there are more keys in 'l' than 'r'.
      continue;
    }
    if (lk > rk) {
      // These edges are in 'r' but not in 'l'.
      redge_count += rvs.size();
      add_edges(missing, rk, rvs);
      rc = sqlite3_step(rstmt.get());
      if (n > m)
        ++show_progress; // We advanced 'r' and there are more keys in 'r' than 'l'.
      continue;
    }

    ledge_count += lvs.size();
    redge_count += rvs.size();

    // Vertices in 'rvs' not in 'lvs' indicate missing edges.
    std::set<std::uint32_t> dests;
    std::copy_if(rvs.begin()
                 , rvs.end()
                 , std::inserter(dests, dests.begin())
                 , [=](auto const& v){ return lvs.find(v) == lvs.end(); } );
    add_edges(missing, lk, dests);
    // Vertices in 'lvs' not in 'rvs' indicate extra edges.
    dests.clear();
    std::copy_if(lvs.begin()
                 , lvs.end()
                 , std::inserter(dests, dests.begin())
                 , [=](auto const& v){ return rvs.find(v) == rvs.end(); } );
    add_edges(extra, rk, dests);

    lc = sqlite3_step(lstmt.get());
    rc = sqlite3_step(rstmt.get());
    ++show_progress; // No matter whether 'l' or 'r' has more keys, progress was made.
  }
  std::cout << "\nResults\n=======\n";
  int digits = std::log10(std::numeric_limits<std::uint32_t>::max()) + 1;
  std::cout << "Edges in 'l': " << ledge_count << "\n";
  std::cout << "Edges in 'r': " << redge_count << "\n";
  std::cout << "Edges in 'r' missing in 'l' (there are " << missing.size() << "):\n";
  for(auto const& edge : missing) {
    std::cout << "  " << std::right << std::setw(digits) << edge.first << " " << edge.second  << '\n';
  }
  std::cout << "Edges in 'l' missing in 'r' (there are " << extra.size() << "):\n";
  for(auto const& edge : extra) {
    std::cout << "  " << std::right << std::setw(digits) << edge.first << " " << edge.second  << '\n';
  }
  std::cout << std::endl;
}

int main(int argc, char const* argv[]) {
  try {
    std::string file;
    po::options_description desc("Allowed options");
    desc.add_options()
      ("help,h", "print usage message")
      ("dump,d", po::value<std::string>(&file), "dump a depgraph")
      ("comp,c", po::value<std::vector<std::string>>(), "compare two depgraphs")
      ;

    po::variables_map vm;
    po::store(parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if(vm.count("help")) {
      std::cout << desc << "\n";

      return 0;
    }

    if(vm.count("dump")) {
      dump_depgraph(file);

      return 0;
    }

    if(vm.count("comp")) {
      std::vector<std::string> files = vm["comp"].as<std::vector<std::string>>();
      if (files.size() != 2) {
        throw std::runtime_error("Error: \"--comp\" must appear exactly twice");
      }
      std::string first_file = files[0], second_file = files[1];
      if(!boost::filesystem::exists(first_file)) {
        throw std::runtime_error("Error: \"" + first_file+ "\" does not exist");
      }
      if(!boost::filesystem::exists(second_file)) {
        throw std::runtime_error("Error: \"" + second_file+ "\" does not exist");
      }

      comp_depgraph(first_file, second_file);

      return 0;
    }
  }
  catch(std::exception& e) {
    std::cerr << e.what() << "\n";

    return 1;
  }

  return 0;
}
