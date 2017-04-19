import numpy as np
import matplotlib.pyplot as plt
import matplotlib
import scipy.stats

def getData(path):
	f = open(path,'r')
	
	allRuns = []
	title = ""
	header = []
	matrix = []
	row = []
	isHeader = False
	
	for line in f:
		columns = line.split(',')
		#Parse the header
		if(len(columns) == 1 and len(columns[0]) > 1):
			if(len(matrix) > 0):
				allRuns.append((title,header,matrix))
				header = []
				matrix = []
			title = columns[0]
			isHeader = True
		else:
			for element in columns:
				textStart = element.find("'")
				textEnd = element.rfind("'")
				text = element[textStart+1:textEnd]
				if(len(text) > 0):
					if(isHeader):
						header.append(text)
					else:
						row.append(text)
				
			if(isHeader):
				isHeader = False
			elif(len(row) > 1):
				matrix.append(row)
				row = []
	
	if(len(matrix) > 0):
		allRuns.append((title,title,matrix))
	
	f.close()
	
	return allRuns

def sortData(data,refcol,globalSort):
	newData = []
	if(globalSort):
		referenceRow = [row[refcol] for row in data[0][2]]
	
	for matrix in data:
		if(not globalSort):
			referenceRow = [row[refcol] for row in matrix[2]]
		matrixData = matrix[2]
		title = matrix[0]
		header = matrix[1]
		sortedCols = []
		sortedData = []
		
		for col in range(len(matrixData[0])):
			column = [row[col] for row in matrixData]
			sortedCol = [x for (y,x) in sorted(zip(referenceRow,column))]
			sortedCols.append(sortedCol)
		
		for row in range(len(sortedCols[0])):
			row = [col[row] for col in sortedCols]
			sortedData.append(row)
		
		newData.append((title,header,sortedData))

	#print(str(newData).replace('[[','\n\n'))
	return newData

def plotSpeedup():
	data = getData('allruns.txt')
	data = sortData(data,0,False)
	
	averages = []
	for mat in data:
		matDat = mat[2]
		col = [float(row[2]) for row in matDat]
		
		averages.append(scipy.stats.hmean(col))
	
	data.remove(data[2])
	data.remove(data[2])
	
	keepOutPlots = [3,4,5,10]
	
	width = 0.15
	N = 11 - len(keepOutPlots)
	ind =  np.arange(N)
	fig,ax = plt.subplots()
	colors = ['r','g','b','y','c','m']
	patterns = ('','-', '.', 'x', '\\', 'o', '*', 'O', '+')
	subBars = []
	titles = []
	ticklabels = []
	index = 0
	
	for row in range(len(data[0][2])):
		if(not(row in keepOutPlots)):
			ticklabels.append(data[0][2][row][0])
	
	titles = ['DCPT two len. patt. matcher','DCPT arb. len. patt. matcher','Strided','TCPRE','Time-delta']
	
	for mat in data:
			matDat = mat[2]
			titles.append(mat[0])
		
			col = []
			for row in range(len(matDat)):
				if(not(row in keepOutPlots)):
					col.append(float(matDat[row][2]))
					
			subBars.append(ax.bar(ind+(width*index),col,width,color=colors[index],hatch=patterns[index]))
			index += 1
	
	ax.set_xticks(ind + (width*5/2))
	ax.set_xticklabels(ticklabels)
	ax.set_title('Speedup of variuos prefetchers')
	ax.set_xlabel('Benchmark')
	ax.set_ylabel('Average speedup')
	
	ax.legend(subBars,titles)
	
	plt.grid(True)
	plt.show()
	
	print(titles)
	print(averages)

def timedelta_logsize_plot():
	td_labels = [16, 32, 64, 128, 256, 512, 1024, 2048]
	td_logsize = [1.055833333, 1.054166667, 1.056666667, 1.073333333, 1.0725, 1.075, 1.075, 1.075]
	str_logsize = [1.059166667, 1.065, 1.065, 1.0725, 1.071666667, 1.0725, 1.0725, 1.0725]
	
	fig,ax = plt.subplots()
	ax.plot(td_labels,td_logsize,'r',td_labels,str_logsize,'b--',linewidth=3)
	ax.set_xscale('log')
	ax.set_xticks(td_labels,td_labels)
	ax.grid()
	ax.set_title('Strided: Average speedup vs table size')
	ax.set_xlabel('Table size')
	ax.set_ylabel('Average speedup')
	ax.legend(['Time delta','Strided'],loc=4)
	plt.setp(ax,xticks = td_labels)
	plt.xticks(td_labels,td_labels)
	plt.show()

def dcpt_logsize_plot():
	labels = [4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048]
	arb_len = [1.055333863, 1.055333863, 1.07467849, 1.085482236, 1.079917538, 1.080579937, 1.078115834, 1.075840225, 1.075840225, 1.075840225]
	two_len = [1.013703408, 1.050078454, 1.071856472, 1.08418497, 1.081299744, 1.087037323, 1.084382375, 1.086424428, 1.086424428, 1.086424428]
	
	fig,ax = plt.subplots()
	ax.plot(labels,arb_len,'r',labels,two_len,'b--',linewidth=3)
	ax.set_xscale('log')
	ax.set_xticks(labels,labels)
	ax.grid()
	ax.set_title('DCPT: Average speedup vs table size')
	ax.set_xlabel('Table size')
	ax.set_ylabel('Average speedup')
	plt.legend(['Arb. len. pattern matcher','Two len. pattern matcher'],loc=4)
	plt.setp(ax,xticks = labels)
	plt.xticks(labels,labels)
	plt.show()

def dcpt_deltacount_plot():
	labels = [2, 4, 8, 16, 32, 64]
	arb_len = [1, 1.046646112, 1.08467622, 1.080579937, 1.032407354, 0.9939204932]
	two_len = [1, 1.061677316, 1.078769202, 1.087037323, 1.074921976, 1.061168976]
	
	fig,ax = plt.subplots()
	ax.plot(labels,arb_len,'r',labels,two_len,'b--',linewidth=3)
	ax.set_xscale('log')
	ax.set_xticks(labels,labels)
	ax.grid()
	ax.set_title('DCPT: Average speedup vs delta count')
	ax.set_xlabel('Number of deltas')
	ax.set_ylabel('Average speedup')
	plt.legend(['Arb. len. pattern matcher','Two len. pattern matcher'],loc=8)
	plt.setp(ax,xticks = labels)
	plt.xticks(labels,labels)
	plt.show()

def getStatistics():
	data = getData('allruns.txt')
	data = sortData(data,0,False)
	
	averages = []
	coverages = []
	accuracies = []
	titles = []
	for mat in data:
		titles.append(mat[0])
		matDat = mat[2]
		cov = [float(row[4]) for row in matDat]
		acc = [float(row[3]) for row in matDat]
		col = [float(row[2]) for row in matDat]
		
		averages.append(scipy.stats.hmean(col))
		coverages.append(np.mean(cov))
		accuracies.append(np.mean(acc))
	
	print(titles)
	print(averages)
	print(coverages)
	print(accuracies)
	
	data.remove(data[2])
	data.remove(data[2])

def main():
	matplotlib.rcParams.update({'font.size': 14})
	matplotlib.rcParams.update({'figure.autolayout': True})
	#plotSpeedup()
	#timedelta_logsize_plot()
	#dcpt_logsize_plot()
	#dcpt_deltacount_plot()
	getStatistics()

main()
