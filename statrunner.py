import subprocess
import glob
import os

#Path to the framework directory. Should be placed here.
frameworkPath = "../"

def runTest(file):
	print("Running: "+' '.join(["cp",file,"prefetcher/prefetcher.cc"]))
	subprocess.call(["cp",file,"prefetcher/prefetcher.cc"])
	print("Running: "+"./compile.sh")
	subprocess.call("./compile.sh")
	print("Running: "+"python test_prefetcher.py")
	proc = subprocess.Popen(["python","test_prefetcher.py"],stdin=subprocess.PIPE,stdout=subprocess.PIPE)
	out,err = proc.communicate()
	
	print(out)

def runTests():
	os.chdir(frameworkPath)
	fileList = []
	for file in glob.glob(frameworkPath + "ComputerArchitecture/*.cc"):
		fileList.append(file)
	
	for file in fileList:
		runTest(file)

def main():
	runTests()
main()
