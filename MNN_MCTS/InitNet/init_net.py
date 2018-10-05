import numpy as np
import math as m
import random as r
from neuralnet import NeuralNet

"""
Build Neural network
"""
net = NeuralNet()
net.build_network(398,300,200,100,65, hidden_type="ReLU", verbose=True)

# for python code
net.save("pieces_net.txt")
# for C code
net.save2("pieces_network.txt")


net = NeuralNet()
net.add_layer("Input", 462)
net.add_layer("ReLU", 300)
net.add_layer("ReLU", 200)
net.add_layer("ReLU", 100)
net.add_layer("Softmax", 12)
# for python code
net.save("pawn_net.txt")
# for C code
net.save2("pawn_network.txt")


net = NeuralNet()
net.add_layer("Input", 462)
net.add_layer("ReLU", 300)
net.add_layer("ReLU", 200)
net.add_layer("ReLU", 100)
net.add_layer("Softmax", 8)
# for python code
net.save("knight_net.txt")
# for C code
net.save2("knight_network.txt")


net = NeuralNet()
net.add_layer("Input", 462)
net.add_layer("ReLU", 300)
net.add_layer("ReLU", 200)
net.add_layer("ReLU", 100)
net.add_layer("Softmax", 28)
# for python code
net.save("rook_net.txt")
# for C code
net.save2("rook_network.txt")


net = NeuralNet()
net.add_layer("Input", 462)
net.add_layer("ReLU", 300)
net.add_layer("ReLU", 200)
net.add_layer("ReLU", 100)
net.add_layer("Softmax", 28)
# for python code
net.save("bishop_net.txt")
# for C code
net.save2("bishop_network.txt")


net = NeuralNet()
net.add_layer("Input", 462)
net.add_layer("ReLU", 300)
net.add_layer("ReLU", 200)
net.add_layer("ReLU", 100)
net.add_layer("Softmax", 56)
# for python code
net.save("queen_net.txt")
# for C code
net.save2("queen_network.txt")


net = NeuralNet()
net.add_layer("Input", 462)
net.add_layer("ReLU", 300)
net.add_layer("ReLU", 200)
net.add_layer("ReLU", 100)
net.add_layer("Softmax", 10)
# for python code
net.save("king_net.txt")
# for C code
net.save2("king_network.txt")
