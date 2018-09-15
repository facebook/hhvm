from __future__ import absolute_import
from __future__ import division
from __future__ import print_function
from __future__ import unicode_literals
from multiprocessing import Pool, Lock, context
import hashlib
import os
import sys
import getopt
import subprocess
import shutil
import tempfile

helpmessage = """fuzz.py -i <inputfile> -g <generations> -f \
<failureThreshold> -p <prob> -v (for verbose) -c (coverage mode) -t <threads>
--timeout <timeout> -l <logfile> --args

Specifics about each argument can be found in hphp/doc/fuzzer. \

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
fuzzerArgs = ""
logfile = ""
logMutex = Lock()
timeOut = 60

def main(argv):
    inputfile = ''
    generations = 1
    threads = 1
    failureThreshold = 1
    prob = .05
    try:
        opts, args = getopt.getopt(argv, "cvhi:g:f:o:p:t:", ["args=",
                                                             "timeout="])
    except getopt.GetoptError as err:
        # print help information and exit:
        print(err)  # will print something like "option -a not recognized"
        print('fuzz.py -i <inputfile> -g <generations> -f <failureThreshold> \
        -p <prob> -v (for verbose) -c (coverage mode) -t <threads> --args \
        -l <logfile>')
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
        elif opt in ("-l"):
            global logfile
            logfile = opt
        elif opt in ("--timeout"):
            global timeOut
            timeOut = int(arg)

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


def log(str):
    global logfile, logMutex
    with logMutex:
        if len(logfile) > 0:
            with open(logfile, "a+") as f:
                f.write(str + "\n")
        else:
            print(str)


def run(inputfile, failureThreshold, generations, prob, threads):
    if(os.path.exists("mutations")):
        shutil.rmtree("mutations")
    if(os.path.exists("mutation_inputs")):
        shutil.rmtree("mutation_inputs")
    if(len(logfile) > 0):
        if(os.path.exists(logfile)):
            os.remove(logfile)
        f = open(logfile, "w+")
        f.close()
    os.mkdir("mutations")
    os.mkdir("mutations/gen0")
    os.mkdir("mutation_inputs")

    print("Generation 0")
    # produce initial mutations from input
    run_generation(inputfile, "mutations/gen0/input0", prob)
    folder = "mutations/gen0"
    for gen in range(1, generations):
        print("Generation " + str(gen))
        if(verbose):
            log("Filtering failures from gen " + str(gen - 1))
        filter_failures(failureThreshold, folder, threads)
        i = 0
        # move all surviving mutations from output of previous generations
        # to input for the subsequent generation
        os.mkdir("mutation_inputs/gen" + str(gen))
        for root, _dirs, files in os.walk("mutations/gen" + str(gen - 1)):
            for filename in files:
                os.rename(root + "/" + filename, "mutation_inputs/gen" +
                          str(gen) + "/input" + str(i) + ".hhas")
                i = i + 1
        i = 0
        os.mkdir("mutations/gen" + str(gen))
        prefix = "mutation_inputs/gen" + str(gen) + "/"
        # fuzz all surviving inputs
        pool = Pool(processes=threads)
        for filename in os.listdir("mutation_inputs/gen" + str(gen)):
            outdir = "mutations/gen" + str(gen) + "/input" + str(i)
            pool.apply_async(run_generation,
                             args=[prefix + filename, outdir, prob])
            i = i + 1
        pool.close()
        pool.join()
        folder = "mutations/gen" + str(gen)

    # Output of final generation, so log all survivors that both pass
    # verification and crash HHVM
    process_candidates(generations, threads)

    for gen in range(0, generations):
        shutil.rmtree("mutations/gen" + str(gen))
    shutil.rmtree("mutation_inputs")


# Run HHVM on a mutation. If it passes verification AND crashes HHVM, log this
def process_candidate(filename):
    (out, errs) = subprocess.Popen(args=[hhvm_dbgo, "-vEval.AllowHhas=1",
                                   "mutations/candidates/" + filename],
                                   stdout=subprocess.PIPE,
                                   stderr=subprocess.PIPE).communicate()
    # this results in a string of the form b'<error message>'
    errors = str(errs)[2:-1]
    if(len(errors) > 0):
        with logMutex:
            with open("mutations/results.txt", 'a+') as outfile:
                outfile.write("HHVM verified mutations/candidates/" +
                              filename +
                              " but wrote the following on stderr:\n " +
                              errors + "\n\n")


# move all survivors of the previous generation to a folder that won't get
# deleted, and then process them to see if they expose a bug
def process_candidates(generations, threads):
    if(verbose):
        log("Filtering candidates")
    filter_failures(0, "mutations/gen" + str(generations - 1), threads)
    i = 0
    os.mkdir("mutations/candidates")
    for root, _dirs, files in os.walk("mutations/gen" + str(generations - 1)):
        for filename in files:
            os.rename(root + "/" + filename, "mutations/candidates/mut" +
                      str(i) + ".hhas")
            i = i + 1

    pool = Pool(processes=threads)
    results = []
    for filename in os.listdir("mutations/candidates"):
        results.append((filename, pool.apply_async(process_candidate,
                                                   args=[filename])))
    pool.close()
    for filename, res in results:
        try:
            global timeOut
            res.get(timeout=timeOut)
        except context.TimeoutError:
            log("HHVM timeout while running " + filename)


# run the fuzzer on all files in a folder
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
        log("Parsing of file " + file + " failed.")
    elif len(errors) != 0:
        log("Error in fuzzer on file " + file + ": " + errors)


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


# produce a map of files to line numbers for all coverage data from a
# mutation
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


# filter out a mutation if it is too broken, is identical to another, or
# optionally does not add new coverage data
def filter_fail(root, mutation, failureThreshold, md5set, covData):
    old_path = os.getcwd()
    path = tempfile.mkdtemp()

    md5sum = md5(root + "/" + mutation)
    if md5sum in md5set:
        if verbose:
            log("File " + root + "/" + mutation + " was identical to a "
                "previously filtered file")
        os.remove(old_path + "/" + root + "/" + mutation)
        shutil.rmtree(path)
        return md5set, covData

    md5set.add(md5sum)

    hhvm = hhvm_dbgo
    hhvm_args = "-vEval.AllowHhas=1"
    if coverage:
        hhvm = hhvm_dbgo_cov
    else:
        # no need to actually execute the file if not in coverage mode
        hhvm_args = hhvm_args + " -m verify"

    # we have to change to a temp directory here because running the process
    # always prints default.profraw to the directory the process was run in, and
    # we don't want parallel filtering jobs to overwrite the data
    os.chdir(path)
    (assembles, errs) = subprocess.Popen(args=[hhvm, hhvm_args,
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
        log("File " + root + "/" + mutation + " failed assembly")
    elif verbose:
        log("File " + root + "/" + mutation + " had " +
                         str(count) + " Verification errors")
    if count > failureThreshold or assemblerError:
        # mutation was too broken, so delete it
        os.remove(old_path + "/" + root + "/" + mutation)
        shutil.rmtree(path)
        return md5set, covData

    if coverage:
        # run llvm commands to take raw profile data and make it parseable
        # by the fuzzer
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

        # compare produced coverage map to existing coverage data
        for f in coveredLines:
            for line in coveredLines[f]:
                if f not in covData:
                    covData[f] = [line]
                    newCoverage = True
                elif line not in covData[f]:
                    covData[f].append(line)
                    newCoverage = True

        if not newCoverage:
            os.remove(old_path + "/" + root + "/" + mutation)
            if(verbose):
                log("File " + root + "/" + mutation + " added no new "
                        "coverage")

    shutil.rmtree(path)
    return md5set, covData


def filter_failures(failureThreshold, folder, threads):
    md5s = set()
    coverageData = {}
    for root, _dirs, mutations in os.walk(folder):
        pool = Pool(processes=threads)
        results = []
        for mutation in mutations:
            results.append((mutation,
                           pool.apply_async(filter_fail,
                                            args=[root, mutation,
                                                  failureThreshold,
                                                  md5s, coverageData])))
        pool.close()
        for mutation, res in results:
            try:
                # update md5s and coverage data with data from the folder
                global timeOut
                md5set, covData = res.get(timeout=timeOut)
                md5s.update(md5set)
                if coverage:
                    for f in covData:
                        for line in covData[f]:
                            if f not in coverageData:
                                coverageData[f] = [line]
                            elif line not in coverageData[f]:
                                coverageData[f].append(line)
            except context.TimeoutError:
                log("HHVM timeout while filtering " + mutation)

    if verbose:
        log("Finished filtering")


if __name__ == "__main__":
    main(sys.argv[1:])
