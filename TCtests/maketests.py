import os
import sys
import subprocess
import shutil
import fileinput



#path to current folder
directory = os.getcwd()+"/"
testpath = directory

    

def copysource():
    sourcefile = "sourcefile.cc"
    shutil.copyfile("../tcpre.cc", sourcefile)


def modifyConstant(filename, constantName, newValue):
    
    searchExp = "#define "+constantName
    replaceExp = searchExp + " " + str(newValue)

    for line in fileinput.input(filename,inplace=1):
        if searchExp in line:
            print replaceExp.rstrip()
        else:
            print line.rstrip()


def createTestfile ( k_entries, m_tagbits, num_ways_pht ):
    
    # copy source file and create test file
    testfile = "tcpre_"+str(k_entries)
    testfile = testfile + "_" + str(m_tagbits)
    testfile = testfile + "_" + str(num_ways_pht)
    testfile = testfile + ".cc"
    
    shutil.copyfile("sourcefile.cc",testfile)

    modifyConstant(testfile, "K_entries", k_entries)  
    modifyConstant(testfile, "M_TAGBITS", m_tagbits)  
    modifyConstant(testfile, "NUM_WAYS_PHT", num_ways_pht)  
 

copysource()

for K in range(2,6):
    for M in range(8,13):
        for WAYS in range ( 4, 19, 2 ):
            createTestfile(K,M,WAYS)




