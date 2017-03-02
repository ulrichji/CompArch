Put this repo inside the framework/ folder to make the scripts function properly. The framework directory is supplied as help to the exercise this
project is about.

The statrunner.py file is highly dependent on directory structure. In order for it to function properly, the repo must be named
ComputerArchitecture and placed in the framework/ directory as described above. When running the file, the working directory must
be framework/ (stand in the framework directory). Then run the file using "python ComputerArchitecture/statrunner.py". The program
will then copy, compile and run all files that end with ".cc" (example: stridedPrefetcher.cc). The output of running the tests is
put in ComputerArchitecture/run_results.txt. Please note that the file "prefetcher.cc" in the prefetcher/ directory will be overwritten
on each run. Please move this file if it contains work in progress.
