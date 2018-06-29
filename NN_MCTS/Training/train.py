import numpy as np
import math as m
from neuralnet import NeuralNet

def rotate_p(vector):
    if len(vector) != 9:
        print "ERROR!\n"
        exit(0)
    rotated = np.zeros(len(vector))
    rotated[0] = vector[2]
    rotated[1] = vector[5]
    rotated[2] = vector[8]
    rotated[3] = vector[1]
    rotated[4] = vector[4]
    rotated[5] = vector[7]
    rotated[6] = vector[0]
    rotated[7] = vector[3]
    rotated[8] = vector[6]
    return rotated

def rotate_board(vector):
    if len(vector) != 18:
        print "ERROR!\n"
        exit(0)
    rotated = np.zeros(len(vector))
    rotated[0] = vector[2]
    rotated[1] = vector[5]
    rotated[2] = vector[8]
    rotated[3] = vector[1]
    rotated[4] = vector[4]
    rotated[5] = vector[7]
    rotated[6] = vector[0]
    rotated[7] = vector[3]
    rotated[8] = vector[6]
    rotated[0+9] = vector[2+9]
    rotated[1+9] = vector[5+9]
    rotated[2+9] = vector[8+9]
    rotated[3+9] = vector[1+9]
    rotated[4+9] = vector[4+9]
    rotated[5+9] = vector[7+9]
    rotated[6+9] = vector[0+9]
    rotated[7+9] = vector[3+9]
    rotated[8+9] = vector[6+9]
    return rotated

"""
Create training set with corresponding target
"""
training_set = []
training_targetp = []
training_targetv = []

# training set
file_in = open("states.dat", 'r')
lines = file_in.readlines()
N = len(lines)
for i in range(N):
    temp = [float(n) for n in lines[i].split()]
    training_set.append(temp)
    temp = rotate_board(temp)
    training_set.append(temp)
    temp = rotate_board(temp)
    training_set.append(temp)
    temp = rotate_board(temp)
    training_set.append(temp)
file_in.close()

# target probabilities
file_in = open("probabilities.dat", 'r')
lines = file_in.readlines()
N = len(lines)
for i in range(N):
    temp = [float(n) for n in lines[i].split()]
    training_targetp.append(temp)
    temp = rotate_p(temp)
    training_targetp.append(temp)
    temp = rotate_p(temp)
    training_targetp.append(temp)
    temp = rotate_p(temp)
    training_targetp.append(temp)
file_in.close()

# target evaluations
file_in = open("z.dat", 'r')
lines = file_in.readlines()
N = len(lines)
for i in range(N):
    temp = [float(n) for n in lines[i].split()]
    training_targetv.append(temp)
    training_targetv.append(temp)
    training_targetv.append(temp)
    training_targetv.append(temp)
file_in.close()

training_set = np.array(training_set)
training_targetp = np.array(training_targetp)
training_targetv = np.array(training_targetv)

"""
Build Neural network
"""
net = NeuralNet()
net.load("net.txt")


"""
Training
"""
# set parameters for training
net.set_training_param(learning_rate=0.001, momentum=0.9, return_error=True, batchsize=50, training_rounds=1, weight_decay=0.0001)
# train
for i in range(60):
    train_error = net.trainOnDataset(training_set, training_targetp, training_targetv)
    loss = net.loss(training_set, training_targetp, training_targetv)
    print ("Iteration: "+str(i+1)+", loss = "+str(loss))
net.set_training_param(learning_rate=0.0005, momentum=0.9, return_error=True, batchsize=50, training_rounds=1, weight_decay=0.0001)
# train
for i in range(60,120):
    train_error = net.trainOnDataset(training_set, training_targetp, training_targetv)
    loss = net.loss(training_set, training_targetp, training_targetv)
    print ("Iteration: "+str(i+1)+", loss = "+str(loss))



"""
Print error during training on file
"""
"""file_out = open("train_error.dat",'w')
for i in range(len(train_error)):
    file_out.write(str(i+1)+" "+str(train_error[i])+"\n")
file_out.close()"""

"""
Save new network
"""
net.save("net_new.txt")
net.save2("network_new.txt")
