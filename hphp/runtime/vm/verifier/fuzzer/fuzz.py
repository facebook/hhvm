from __future__ import absolute_import
from __future__ import division
from __future__ import print_function
from __future__ import unicode_literals
from concurrent.futures import ThreadPoolExecutor
from threading import RLock
import hashlib
import os
import sys
import getopt
import subprocess
import shutil
import tempfile

helpmessage = """fuzz.py -i <inputfile> -g <generations> -f \
<failureThreshold> -p <prob> -v (for verbose) -c (coverage mode) -t <threads>
--args

Generations specifies the number of generations for which to run \
the fuzzer. Time to run increases exponentially with each generation.

Failure Threshold specifies the maximum number of verifier \
failures a program can have to not be culled at the end of a \
generation. A higher number means more programs are kept,  leading \
to a longer runtime.

Prob specifies the probability of a mutation occuring at each \
point in the input program. A higher number yields more mutations \
per program, and thus a higher chance for verification errors.

Threads specifies the maximum size of the threadpool used to \
run the fuzzer.

When run in coverage mode, (-c), the fuzzer will use coverage data from \
HHVM as part of its genetic algorithm.

Additional arguments can be passed directly to the fuzzer with --args.

Results of a fuzzer run can be found in mutations/results.txt"""

home = os.path.expanduser('~')
fbcode = home + "/fbsource/fbcode"
hphp = fbcode + "/hphp"

fuzzer = fbcode + "/buck-out/dbgo/bin/hphp/" \
    "runtime/vm/verifier/fuzzer/fuzzer/fuzzer.opt"
hhvm_dbgo = fbcode + "/buck-out/dbgo/gen/hphp/" \
    "hhvm/hhvm/hhvm"
hhvm_dbgo_cov = fbcode + "/buck-out/dbgo-cov/gen/hphp/" \
    "hhvm/hhvm/hhvm"


fuzzer_build = "buck build @mode/dbgo //hphp/runtime/vm/verifier/fuzzer:fuzzer"

hhvm_build_dbgo_cov = "buck build @mode/dbgo-cov -c " \
    "\'cxx.coverage_only=hphp/runtime/vm\' //hphp/hhvm:hhvm"

hhvm_build_dbgo = "buck build @mode/dbgo //hphp/hhvm:hhvm"

profdata = fbcode + "/third-party2/llvm-fb/stable/gcc-5-glibc-2.23/" \
    "03859b5/bin/llvm-profdata merge -sparse"

cov = fbcode + "/third-party2/llvm-fb/stable/gcc-5-glibc-2.23/" \
    "03859b5/bin/llvm-cov"

verbose = False
coverage = False
coverageData = {}
fuzzerArgs = ""
mutex = RLock()
md5s = []

def main(argv):
    inputfile = ''
    generations = 1
    threads = 1
    failureThreshold = 1
    prob = .05
    try:
        opts, args = getopt.getopt(argv, "cvhi:g:f:o:p:t:", ["args="])
    except getopt.GetoptError as err:
        # print help information and exit:
        print(err)  # will print something like "option -a not recognized"
        print('fuzz.py -i <inputfile> -g <generations> -f <failureThreshold> \
        -p <prob> -v (for verbose) -c (coverage mode) -t <threads> --args')
        sys.exit(2)
    for opt, arg in opts:
        if opt == '-h':
            print(helpmessage)
            sys.exit()
        elif opt == '-v':
            global verbose
            verbose = True
        elif opt in ("-i"):
            inputfile = arg
        elif opt in ("-c"):
            global coverage
            coverage = True
        elif opt in ("-t"):
            threads = int(arg)
        elif opt in ("-g"):
            generations = int(arg)
        elif opt in ("-f"):
            failureThreshold = int(arg)
        elif opt in ("-p"):
            prob = float(arg)
        elif opt in ("--args"):
            global fuzzerArgs
            fuzzerArgs = arg

    if not os.path.exists(fuzzer):
        print("Compiling Fuzzer")
        os.system(fuzzer_build)
    if not os.path.exists(hhvm_dbgo):
        print("Compiling HHVM")
        os.system(hhvm_build_dbgo)
    if coverage and not os.path.exists(hhvm_dbgo_cov):
        print("Compiling HHVM in coverage mode")
        os.system(hhvm_build_dbgo_cov)
    if (generations < 1):
        print("Generations cannot be less than 1")
        sys.exit(2)

    run(inputfile, failureThreshold, generations, prob, threads)


def run(inputfile, failureThreshold, generations, prob, threads):
    if(os.path.exists("mutations")):
        shutil.rmtree("mutations")
    if(os.path.exists("mutation_inputs")):
        shutil.rmtree("mutation_inputs")
    os.mkdir("mutations")
    os.mkdir("mutations/gen0")
    os.mkdir("mutation_inputs")

    print("Generation 0")
    run_generation(inputfile, "mutations/gen0/input0", prob)
    folder = "mutations/gen0"
    for gen in range(1, generations):
        print("Generation " + str(gen))
        if(verbose):
            print("Filtering failures from gen " + str(gen - 1))
        filter_failures(failureThreshold, folder, threads)
        i = 0
        os.mkdir("mutation_inputs/gen" + str(gen))
        for root, _dirs, files in os.walk("mutations/gen" + str(gen - 1)):
            for filename in files:
                os.rename(root + "/" + filename, "mutation_inputs/gen" +
                          str(gen) + "/input" + str(i) + ".hhas")
                i = i + 1
        i = 0
        os.mkdir("mutations/gen" + str(gen))
        prefix = "mutation_inputs/gen" + str(gen) + "/"
        executor = ThreadPoolExecutor(max_workers=threads)
        for filename in os.listdir("mutation_inputs/gen" + str(gen)):
            outdir = "mutations/gen" + str(gen) + "/input" + str(i)
            executor.submit(run_generation, prefix + filename, outdir, prob)
            # run_generation(prefix + filename, outdir, prob)
            i = i + 1
        executor.shutdown(wait=True)
        folder = "mutations/gen" + str(gen)

    process_candidates(generations, threads)

    for gen in range(0, generations):
        shutil.rmtree("mutations/gen" + str(gen))
    shutil.rmtree("mutation_inputs")


def process_candidate(filename, outfile):
    (out, errs) = subprocess.Popen(args=[hhvm_dbgo, "-vEval.AllowHhas=1",
                                   "mutations/candidates/" + filename],
                                   stdout=subprocess.PIPE,
                                   stderr=subprocess.PIPE).communicate()
    # this results in a string of the form b'<error message>'
    errors = str(errs)[2:-1]
    if(len(errors) > 0):
        outfile.write("HHVM verified mutations/candidates/" + filename +
                      " but wrote the following on stderr:\n " + errors +
                      "\n\n")


def process_candidates(generations, threads):
    if(verbose):
        print("Filtering candidates")
    outfile = open("mutations/results.txt", 'w')
    filter_failures(0, "mutations/gen" + str(generations - 1), threads)
    i = 0
    os.mkdir("mutations/candidates")
    for root, _dirs, files in os.walk("mutations/gen" + str(generations - 1)):
        for filename in files:
            os.rename(root + "/" + filename, "mutations/candidates/mut" +
                      str(i) + ".hhas")
            i = i + 1

    executor = ThreadPoolExecutor(max_workers=threads)
    for filename in os.listdir("mutations/candidates"):
        executor.submit(process_candidate, filename, outfile)
        # process_candidate(filename, outfile)
    executor.shutdown(wait=True)
    outfile.close()


def run_generation(file, folder, prob):
    os.mkdir(folder)
    args = [fuzzer, file, "-o", folder,
                            "-prob", str(prob)]
    if len(fuzzerArgs) > 0:
        args.extend(fuzzerArgs.split(" "))

    (out, errs) = subprocess.Popen(args, stdout=subprocess.PIPE,
                                   stderr=subprocess.PIPE).communicate()
    errors = str(errs)[2:-1]
    if "Parsing" in errors:
        print("Parsing of file " + file + " failed.")
    elif len(errors) != 0:
        print("Error in fuzzer on file " + file + ": " + errors)


def md5(fname):
    hash_md5 = hashlib.md5()
    with open(fname, "rb") as f:
        for chunk in iter(lambda: f.read(4096), b""):
            hash_md5.update(chunk)
    return hash_md5.hexdigest()


def lineNumber(line):
    i = 0
    numStr = ""
    while line[i].isdigit():
        numStr = numStr + line[i]
        i = i + 1
    num = int(numStr)
    line = line[i + 1:].strip()
    if line[0] == "|" or line[0] == "0":
        return num, False
    return num, True


def aggregateCoverage(folder):
    ret = {}
    for subdir, _dirs, files in os.walk(folder):
        for file in files:
            aggregateFileCoverage(subdir + "/" + file, ret)
    return ret


def aggregateFileCoverage(file, ret):
    with open(file) as f:
        covData = f.readlines()
        curFn = ""
        for line in covData:
            line = line.strip()
            if len(line) == 0 or "Coverage report" in line or "Created" in line:
                continue
            elif str(line[0]).isdigit():
                num, covered = lineNumber(line)
                if curFn in ret and covered:
                    ret[curFn].append(num)
                elif covered:
                    ret[curFn] = [num]
            elif line[0] == "|" or line[0] == "-":
                continue
            else:
                curFn = line

        return ret


def listdir_fullpath(d):
    return [os.path.join(d, f) for f in os.listdir(d)]


def filter_fail(root, mutation, failureThreshold):
    old_path = os.getcwd()
    path = tempfile.mkdtemp()

    md5sum = md5(root + "/" + mutation)
    global md5s
    if md5sum in md5s:
        if verbose:
            print("File " + root + "/" + mutation + " was identical to a "
                  "previously filtered file")
        os.remove(old_path + "/" + root + "/" + mutation)
        shutil.rmtree(path)
        return

    global mutex
    with mutex:
        md5s.append(md5sum)

    hhvm = hhvm_dbgo
    if coverage:
        hhvm = hhvm_dbgo_cov

    # we have to change to a temp directory here because running the process
    # always prints default.profraw to the directory the process was run in, and
    # we don't want parallel filtering jobs to overwrite the
    # print(hhvm)
    os.chdir(path)
    (assembles, errs) = subprocess.Popen(args=[hhvm, "-vEval.AllowHhas=1",
                                         old_path + "/" + root + "/" +
                                         mutation], stdout=subprocess.PIPE,
                                         stderr=subprocess.PIPE).communicate()
    os.chdir(old_path)

    errs = str(errs)
    assembles = str(assembles)
    output = errs.split("Verification Error")
    count = len(output) - 1
    assemblerError = "Assembler Error" in assembles
    if verbose and assemblerError:
        print("File " + root + "/" + mutation + " failed assembly")
    elif verbose:
        print("File " + root + "/" + mutation + " had " +
                         str(count) + " Verification errors")
    if count > failureThreshold or assemblerError:
        # mutation was too broken, so delete it
        # os.remove(old_path + "/" + root + "/" + mutation)
        shutil.rmtree(path)
        return

    if coverage:
        command = profdata + " " + path + "/default.profraw -o " + path
        command = command + "/default.merged"
        os.system(command)
        subprocess.Popen(args=[cov, "show", "--format=text", "-output-dir",
                               path + "/coverage", "-instr-profile",
                               path + "/default.merged", hhvm],
                               stdout=subprocess.PIPE,
                               stderr=subprocess.PIPE).communicate()
        coveredLines = aggregateCoverage(path + "/coverage")
        newCoverage = False
        global coverageData
        for f in coveredLines:
            for line in coveredLines[f]:
                if f not in coverageData:
                    with mutex:
                        coverageData[f] = [line]
                    newCoverage = True
                elif line not in coverageData[f]:
                    with mutex:
                        coverageData[f].append(line)
                    newCoverage = True

        if not newCoverage:
            # os.remove(old_path + "/" + root + "/" + mutation)
            if(verbose):
                print("File " + root + "/" + mutation + " added no new "
                        "coverage")

    shutil.rmtree(path)


def filter_failures(failureThreshold, folder, threads):
    # executor = ThreadPoolExecutor(max_workers=threads)
    for root, _dirs, mutations in os.walk(folder):
        for mutation in mutations:
            # executor.submit(filter_fail, root, mutation, failureThreshold)
            filter_fail(root, mutation, failureThreshold)
    # executor.shutdown(wait=True)
    if verbose:
        print("Finished filtering")


if __name__ == "__main__":
    main(sys.argv[1:])
