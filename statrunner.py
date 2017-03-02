import subprocess
import glob
import os
import sys

#Path to the framework directory. Should be placed here.
directory = os.getcwd()+"/"
frameworkPath = directory

def runTest(file,comment=''):
	outputFile = "prefetcher/prefetcher.cc"
	print("Running: "+' '.join(["cp",file,outputFile]))
	subprocess.call(["cp",file,outputFile])
	
	compileProgram = "./compile.sh"
	print("Running: "+compileProgram)
	subprocess.call(compileProgram)
	
	testProgram = "python test_prefetcher.py"
	print("Running: "+testProgram)
	if(comment != ''):
		print(comment)
	proc = subprocess.Popen(["python","test_prefetcher.py"],stdin=subprocess.PIPE,stdout=subprocess.PIPE)
	out,err = proc.communicate()
	
	return out,err
	
	#print(out)

def extractFileName(file):
	lastSlash = file.rfind("/")
	return file[lastSlash+1:]

def getRowValues(row):
	return row.split()

def extractData(text):
	lines = text.split("\n")
	columns = []
	resultTable = []
	numberOfDottedLines = 0
	for line in lines:
		if("------------" in line and numberOfDottedLines < 3):
			numberOfDottedLines += 1
		elif(numberOfDottedLines == 1):
			columns = getRowValues(line)
		elif(numberOfDottedLines > 0 and numberOfDottedLines < 3):
			row = getRowValues(line)
			resultTable.append(row)
	
	return columns,resultTable
	
def getTestText():
	f = open(directory+"test.txt","r")
	text = ""
	for line in f:
		text += line
	f.close()
	return text

def runTests():
	os.chdir(frameworkPath)
	fileList = []
	
	resultFile = open(frameworkPath + "ComputerArchitecture/run_results.txt","w")
	
	for file in glob.glob(frameworkPath + "ComputerArchitecture/*.cc"):
		fileList.append(file)
	
	fileCounter = 1
	totalFiles = len(fileList)
	
	for file in fileList:
		testText = "Running test "+str(fileCounter)+" of "+str(totalFiles)+":"
		print(testText)
		fileCounter += 1
		
		output,error = runTest(file,comment=testText)
		#output = getTestText()
		fileName = extractFileName(file)
		column,data = extractData(output)
		resultFile.write(fileName + ":\n"+str(column)+"\n"+"\n".join(str(i)[1:len(str(i))-1] for i in data))
		resultFile.write('\n\n')
	
	resultFile.close()

def main():
	runTests()
main()
