"""
Test Example: A Neural Network to classify
points on a plane (3 categories) + associate to them
a score value
"""

import numpy as np
import math as m
import random as r
from neuralnet import NeuralNet

r.seed(99)
np.random.seed(99)
"""
Create training set with corresponding target
"""
training_set = []
training_targetp = []
training_targetv = []
training_label = []

for i in range(100):
    x = r.uniform(0.0, 1.0)
    y = r.uniform(0.0, 1.0)
    vector = [x,y]
    p = [1.0, 0.0, 0.0]
    v = 1.0
    training_set.append(vector)
    training_targetp.append(p)
    training_targetv.append(v)
    training_label.append(0)
for i in range(100):
    x = r.uniform(1.5, 2.5)
    y = r.uniform(0.0, 1.0)
    vector = [x,y]
    p = [0.0, 1.0, 0.0]
    v = 0.0
    training_set.append(vector)
    training_targetp.append(p)
    training_targetv.append(v)
    training_label.append(1)
for i in range(100):
    x = r.uniform(0.5, 2.0)
    y = r.uniform(2.0, 3.0)
    vector = [x,y]
    p = [0.0, 0.0, 1.0]
    v = -1.0
    training_set.append(vector)
    training_targetp.append(p)
    training_targetv.append(v)
    training_label.append(2)

training_set = np.array(training_set)
training_targetp = np.array(training_targetp)
training_targetv = np.array(training_targetv)
training_label = np.array(training_label)

file_out = open("set.dat",'w')
for i in range(len(training_label)):
    file_out.write(str(training_set[i][0])+" "+str(training_set[i][1])+" "+str(training_label[i])+"\n")
file_out.close()


"""
Build Neural network
"""
net = NeuralNet()
net.build_network(2,4,4,4, hidden_type="Tanh", verbose=True)


"""
Training
"""
# set parameters for training
net.set_training_param(learning_rate=0.1, momentum=0.9, return_error=True, batchsize=10, training_rounds=10)
# train
train_error = net.trainOnDataset(training_set, training_targetp, training_targetv)


"""
Print error during training on file
"""
file_out = open("train_error.dat",'w')
for i in range(len(train_error)):
    file_out.write(str(i+1)+" "+str(train_error[i])+"\n")
file_out.close()


"""
Compute loss on training set
"""
loss = net.loss(training_set, training_targetp, training_targetv)
print ("Train loss = "+str(loss))


"""
Compute score on training set
and print on file actual and target outputs for v
"""
score = 0.0
file_out = open("v.dat",'w')
for i in range(len(training_label)):
    p,v = net.activate(training_set[i])
    label = np.argmax(p)
    file_out.write(str(i+1)+" "+str(training_targetv[i])+" "+str(v)+"\n")
    if label == training_label[i]:
        score += 1.0
score /= len(training_label)
print ("Train score = "+str(score))
file_out.close()

"""
Save network on file for future use
"""
# for python code
net.save("net.txt")
# for C code
net.save2("network.txt")

"""
Load saved network
"""
#net2 = NeuralNet()
#net2.load("net.txt")
