import re
import math
import numpy
import sys

#cheking if the user put the input file name as argument
if len(sys.argv) != 2:
    #if not print a message with the correct usage of the program
    print '**** ERROR ****'
    print 'The program has to be used as follows:'
    print 'python pca.py input_file_name'
    exit(1)
file_name = sys.argv[1]
# open input file
file_in = open(file_name, 'r')
# create a list to store the input
input = []
# read file and add data to the list input
# each row contains a configuration of spins
lines = file_in.readlines()
Nconfs = len(lines) # number of configurations
for i in range(Nconfs):
    temp = map(float, lines[i].split())
    input.append([])
    for j in range(len(temp)):
        input[i].append(temp[j])
file_in.close()
# build a numpy array with the configurations
# numpy arrays are more efficient!
confs = numpy.array(input)
# transpose the matrix with the configurations
# this is needed only to build the covariance matrix
confs_t = numpy.array(confs).T
# build the covariance matrix
cov_matrix = numpy.cov(confs_t)
# find eigenvectors and eigenvalues of the cov_matrix
eigenValues, eigenVectors = numpy.linalg.eig(cov_matrix)
# sort eigenvalues and eigenvectors
# from highest to lowest
idx = eigenValues.argsort()[::-1]
eigenValues = eigenValues[idx]
eigenVectors = eigenVectors[:,idx]
# create a list to store the projections of the
# configurations on the 1st 3 principal components
projection = numpy.zeros((Nconfs,3))
# open file for saving output
file_out = open("results.dat",'w')
for i in range(Nconfs):
    file_out.write(str(i)+" ")
    for j in range(3):
        # project configuration i on component j
        projection[i][j] = numpy.dot(confs[i], eigenVectors[:,j])
        file_out.write(str(projection[i][j])+" ")
    file_out.write("\n")
file_out.close()
# normalize eigenvalues and write them on file
denominator = numpy.sum(eigenValues)
normalized_eigenValues = eigenValues/denominator
file_out = open("norm_eigen.dat",'w')
for i in range(len(normalized_eigenValues)):
    file_out.write(str(i+1)+" "+str(normalized_eigenValues[i])+"\n")
file_out.close()
