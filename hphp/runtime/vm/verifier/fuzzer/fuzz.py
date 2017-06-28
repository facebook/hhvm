from __future__ import absolute_import
from __future__ import division
from __future__ import print_function
from __future__ import unicode_literals
from concurrent.futures import ThreadPoolExecutor
import os
import sys
import getopt
import subprocess
import shutil

helpmessage = """fuzz.py -i <inputfile> -g <generations> -f \
<failureThreshold> -p <prob> -v (for verbose) -t <threads>

Generations specifies the number of generations for which to run \
the fuzzer. Time to run increases exponentially with each generation.

Failure Threshold specifies the maximum number of verifier \
failures a program can have to not be culled at the end of a \
generation. A higher number means more programs are kept, leading \
to a longer runtime.

Prob specifies the probability of a mutation occuring at each \
point in the input program. A higher number yields more mutations \
per program, and thus a higher chance for verification errors.

Threads specifies the maximum size of the threadpool used to \
run the fuzzer."""

fuzzer = os.path.expanduser('~') + "/fbsource/fbcode/buck-out/bin/hphp/" \
    "runtime/vm/verifier/fuzzer/fuzzer/fuzzer"
hhvm = os.path.expanduser('~') + "/fbsource/fbcode/buck-out/gen/hphp/" \
    "hhvm/hhvm/hhvm"

verbose = False


def main(argv):
    inputfile = ''
    generations = 1
    threads = 1
    failureThreshold = 2
    prob = .05
    try:
        opts, args = getopt.getopt(argv, "vhi:g:f:o:p:t:")
    except getopt.GetoptError:
        print('fuzz.py -i <inputfile> -g <generations> -f <failureThreshold> \
        -p <prob> -v (for verbose) -t <threads>')
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
        elif opt in ("-t"):
            threads = int(arg)
        elif opt in ("-g"):
            generations = int(arg)
        elif opt in ("-f"):
            failureThreshold = int(arg)
        elif opt in ("-p"):
            prob = float(arg)

    if(not os.path.exists(fuzzer)):
        print("Please compile the fuzzer before using this tool")
        sys.exit(2)
    if(not os.path.exists(hhvm)):
        print("Please compile hhvm before using this tool")
        sys.exit(2)
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
                          str(gen) + "/input" + str(i))
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


def process_candidate(filename):
    data = subprocess.Popen(args=[hhvm, "-vEval.AllowHhas=1",
                            "mutations/candidates/" + filename],
                            stdout=subprocess.PIPE,
                            stderr=subprocess.PIPE)
    errors = str(data.stderr.read())
    # this results in a string of the form b'<error message>'
    errors = errors[2:]
    errors = errors[:-1]
    if(len(errors) > 0):
        print("HHVM verified mutations/candidates/" + filename +
              " but wrote the following on stderr:\n " + errors)


def process_candidates(generations, threads):
    if(verbose):
        print("Filtering candidates")
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
        executor.submit(process_candidate, filename)
    executor.shutdown(wait=True)


def run_generation(file, folder, prob):
    os.mkdir(folder)
    subprocess.run(args=[fuzzer, file, "-o", folder,
                           "-prob", str(prob), "-immediate", "1",
                           "-metadata", "1",
                           "-duplicate", "1", "-remove", "1", "-reorder", "0"])


def filter(root, mutation, failureThreshold):
    data = subprocess.Popen(args=[hhvm, "-vEval.AllowHhas=1", "-m",
                            "verify", root + "/" + mutation],
                            stdout=subprocess.PIPE,
                            stderr=subprocess.PIPE)
    output = str(data.stderr.read()).split("Verification Error")
    count = len(output) - 1
    assemblerError = "Assembler Error" in str(data.stderr.read())
    if(verbose):
        print("File " + root + "/" + mutation + " had " + str(count) +
               " errors")
    if(count > failureThreshold or assemblerError):
        # mutation was too broken, so delete it
        os.remove(root + "/" + mutation)


def filter_failures(failureThreshold, folder, threads):
    executor = ThreadPoolExecutor(max_workers=threads)
    for root, _dirs, mutations in os.walk(folder):
        for mutation in mutations:
            executor.submit(filter, root, mutation, failureThreshold)
    executor.shutdown(wait=True)


if __name__ == "__main__":
    main(sys.argv[1:])
