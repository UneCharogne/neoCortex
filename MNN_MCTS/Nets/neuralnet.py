import numpy as np
import math as m
import random
from layer import Layer
from layer import Linear
from layer import Tanh
from layer import Sigmoid
from layer import Softmax
from layer import SoftSign
from layer import ReLU

class NeuralNet (object):
    """
    A class to represent a feed-forward neural network
    """
    def __init__(self):
        self.layers = []
        self.Nlayers = 0
        self.out_type = None
        self.hidden_type = None
        self.solver = "sgd"
        self.learning_rate = 0.01
        self.momentum = 0.0
        self.beta1 = 0.9
        self.beta2 = 0.999
        self.epsilon = 1e-08
        self.weight_decay = 0.0
        self.batchsize = 1
        self.training_rounds = 1
        self.verbose = True
        self.scope = None
        self.error_list = []
        self.return_error = False


    def build_network(self, *d, **types):
        """
        Method to build a neural network structure
        """
        # check number of layers
        Nlayers = len(d)
        if Nlayers<2:
            print ("ERROR: A neural network needs at least an input and an output layer!")
            exit(1)
        if types.has_key("verbose"):
            self.verbose = types.get("verbose")
        # check if the user specified the scope of the network
        # if not set it to classification
        if types.has_key("scope"):
            self.scope = types.get("scope")
        else:
            self.scope = "Classification"
        # check if the user specified the types of layers
        # if not set to default types
        if types.has_key("out_type"):
            self.out_type = types.get("out_type")
        else:
            if d[Nlayers-1]==1:
                self.out_type = "Linear"
            else:
                self.out_type = "Softmax"
        if types.has_key("hidden_type"):
            self.hidden_type = types.get("hidden_type")
        else:
            if Nlayers>2:
                self.hidden_type = "Tanh"
        # add layers to the neural network
        # add input layers
        self.layers.append(Layer(d[0]))
        # if present, add hidden layers
        if Nlayers>2:
            if self.hidden_type == "Tanh":
                for i in range(1,Nlayers-1):
                    self.layers.append(Tanh(d[i],d[i-1]))
                    self.layers[i].xavier_init_weights()
            elif self.hidden_type == "Sigmoid":
                for i in range(1,Nlayers-1):
                    self.layers.append(Sigmoid(d[i],d[i-1]))
                    self.layers[i].xavier_init_weights()
            elif self.hidden_type == "Linear":
                for i in range(1,Nlayers-1):
                    self.layers.append(Linear(d[i],d[i-1]))
                    self.layers[i].xavier_init_weights()
            elif self.hidden_type == "Softmax":
                for i in range(1,Nlayers-1):
                    self.layers.append(Softmax(d[i],d[i-1]))
                    self.layers[i].xavier_init_weights()
            elif self.hidden_type == "SoftSign":
                for i in range(1,Nlayers-1):
                    self.layers.append(SoftSign(d[i],d[i-1]))
                    self.layers[i].xavier_init_weights()
            elif self.hidden_type == "ReLU":
                for i in range(1,Nlayers-1):
                    self.layers.append(ReLU(d[i],d[i-1]))
                    self.layers[i].xavier_init_weights()
            else:
                print ("ERROR: no layer with "+str(self.hidden_type)+" exist!")
                exit(1)
        # add output layer
        if self.out_type == "Softmax":
            self.layers.append(Softmax(d[Nlayers-1],d[Nlayers-2]))
            self.layers[Nlayers-1].xavier_init_weights()
        elif self.out_type == "Sigmoid":
            self.layers.append(Sigmoid(d[Nlayers-1],d[Nlayers-2]))
            self.layers[Nlayers-1].xavier_init_weights()
        elif self.out_type == "Linear":
            self.layers.append(Linear(d[Nlayers-1],d[Nlayers-2]))
            self.layers[Nlayers-1].xavier_init_weights()
        elif self.out_type == "Tanh":
            self.layers.append(Tanh(d[Nlayers-1],d[Nlayers-2]))
            self.layers[Nlayers-1].xavier_init_weights()
        elif self.out_type == "SoftSign":
            self.layers.append(SoftSign(d[Nlayers-1],d[Nlayers-2]))
            self.layers[Nlayers-1].xavier_init_weights()
        elif self.out_type == "ReLU":
            self.layers.append(ReLU(d[Nlayers-1],d[Nlayers-2]))
            self.layers[Nlayers-1].xavier_init_weights()
        else:
            print ("ERROR: no layer with "+str(self.out_type)+" exist!")
            exit(1)
        #save number of layers
        self.Nlayers = Nlayers
        if self.verbose:
            self.print_network_structure()


    def add_layer(self, type, dim):
        """
        Method that adds to the network a layer
        of dimension dim and type type
        """
        if type=="Input":
            if self.Nlayers==0:
                self.layers.append(Layer(dim))
                self.Nlayers = len(self.layers)
            else:
                print ("ERROR: the network already has an input layer!")
                exit(1)
        elif type=="Linear":
            if self.Nlayers==0:
                print ("ERROR: the network needs an input layer first!")
                exit(1)
            else:
                self.layers.append(Linear(dim,self.layers[self.Nlayers-1].n))
                self.Nlayers = len(self.layers)
                self.layers[self.Nlayers-1].xavier_init_weights()
        elif type=="Tanh":
            if self.Nlayers==0:
                print ("ERROR: the network needs an input layer first!")
                exit(1)
            else:
                self.layers.append(Tanh(dim,self.layers[self.Nlayers-1].n))
                self.Nlayers = len(self.layers)
                self.layers[self.Nlayers-1].xavier_init_weights()
        elif type=="ReLU":
            if self.Nlayers==0:
                print ("ERROR: the network needs an input layer first!")
                exit(1)
            else:
                self.layers.append(ReLU(dim,self.layers[self.Nlayers-1].n))
                self.Nlayers = len(self.layers)
                self.layers[self.Nlayers-1].xavier_init_weights()
        elif type=="SoftSign":
            if self.Nlayers==0:
                print ("ERROR: the network needs an input layer first!")
                exit(1)
            else:
                self.layers.append(SoftSign(dim,self.layers[self.Nlayers-1].n))
                self.Nlayers = len(self.layers)
                self.layers[self.Nlayers-1].xavier_init_weights()
        elif type=="Sigmoid":
            if self.Nlayers==0:
                print ("ERROR: the network needs an input layer first!")
                exit(1)
            else:
                self.layers.append(Sigmoid(dim,self.layers[self.Nlayers-1].n))
                self.Nlayers = len(self.layers)
                self.layers[self.Nlayers-1].xavier_init_weights()
        elif type=="Softmax":
            if self.Nlayers==0:
                print ("ERROR: the network needs an input layer first!")
                exit(1)
            else:
                self.layers.append(Softmax(dim,self.layers[self.Nlayers-1].n))
                self.Nlayers = len(self.layers)
                self.layers[self.Nlayers-1].xavier_init_weights()
        else:
            print ("ERROR: no such layer available!")
            exit(1)


    def set_scope(self, scope):
        if scope=="Classification" or scope=="Regression":
            self.scope = scope
        else:
            print ("Error: scope must be Classification or Regression!")
            exit(1)


    def print_network_structure(self):
        """
        Method that prints on screen the neural network structure
        """
        print ("\n**********    Neural Network Structure    **********")
        print ("Scope of the network:       "+str(self.scope))
        if self.Nlayers>0:
            print ("Input layer of dimesion:    " + str(self.layers[0].n))
        else:
            print ("The network is empty")
        if self.Nlayers>2:
            for i in range(1,self.Nlayers-1):
                print ("Hidden layer of dimension:  "+str(self.layers[i].n)+ \
                " ("+str(self.layers[i].type)+")")
        if self.Nlayers>1:
            print ("Output layer of dimension:  "+str(self.layers[self.Nlayers-1].n)+ \
            " ("+str(self.layers[self.Nlayers-1].type)+")")


    def set_training_param(self, **params):
        """
        Method to set the parameters for training:
        - solver
        - learning_rate
        - momentum
        - beta1
        - beta2
        - epsilon
        - weight_decay
        - batchsize
        - training_rounds
        - return_error
        If not called default values are used
        """
        if params.has_key("solver"):
            self.solver = params.get("solver")
            if (self.solver!="sgd" and self.solver!="adam"):
                print ('ERROR: solver "'+self.solver+'" not available!')
                exit(1)
        if params.has_key("learning_rate"):
            self.learning_rate = params.get("learning_rate")
        if params.has_key("momentum"):
            self.momentum = params.get("momentum")
        if params.has_key("beta1"):
            self.beta1 = params.get("beta1")
        if params.has_key("beta2"):
            self.beta2 = params.get("beta2")
        if params.has_key("epsilon"):
            self.epsilon = params.get("epsilon")
        if params.has_key("weight_decay"):
            self.weight_decay = params.get("weight_decay")
        if params.has_key("batchsize"):
            self.batchsize = params.get("batchsize")
            if self.batchsize<0:
                print ('ERROR: batchsize must be larger than 0!')
                exit(1)
        if params.has_key("training_rounds"):
            self.training_rounds = params.get("training_rounds")
            if self.training_rounds<0:
                print ('ERROR: training_rounds must be larger than 0!')
                exit(1)
        if params.has_key("return_error"):
            self.return_error = params.get("return_error")
        if self.verbose:
            print "\n**********     Parameters for training    **********"
            print ("Solver:           "+ str(self.solver))
            print ("Learning rate:    "+ str(self.learning_rate))
            if self.solver=="sgd":
                print ("Momentum:         "+ str(self.momentum))
            if self.solver=="adam":
                print ("Beta1:            "+ str(self.beta1))
                print ("Beta2:            "+ str(self.beta2))
                print ("Epsilon:          "+ str(self.epsilon))
            print ("Weight decay:     "+ str(self.weight_decay))
            print ("Batch size:       "+ str(self.batchsize))
            print ("Training rounds:  "+ str(self.training_rounds))
            print ("Return error:     "+ str(self.return_error))


    def forward_propagation(self, input_vector):
        """
        Method that produces the network output
        of input_vector
        """
        self.layers[0].activate(input_vector)
        for i in range(1,self.Nlayers):
            self.layers[i].activate(self.layers[i-1])


    def back_propagation(self, target):
        """
        Method that backpropagate the error trough
        the layers of the network
        """
        # output layer
        self.layers[self.Nlayers-1].delta_out(target)
        self.layers[self.Nlayers-1].update_gradients(self.layers[self.Nlayers-2])
        # hidden layers (if present)
        for i in range(self.Nlayers-2,0,-1):
            if self.layers[i+1].type=="Linear":
                self.layers[i].delta(self.layers[i+1])
            else:
                self.layers[i].delta(self.layers[i+1].lin)
            self.layers[i].update_gradients(self.layers[i-1])


    def batch_train_sgd(self, batch, target,  time):
        # batch size
        nbatch = len(batch)
        # error
        error = 0.0
        for n in range(nbatch):
            # produce output
            self.forward_propagation(batch[n])
            # compute error
            error += self.layers[self.Nlayers-1].compute_error(target[n])
            # back propagate the error
            self.back_propagation(target[n])
        # update weights (batch gradient-descent+momentum)
        for i in range(1,self.Nlayers):
            self.layers[i].gradient_descent(self.learning_rate, \
            self.momentum, nbatch, self.weight_decay)
        #add error to error_list
        error /= nbatch
        self.error_list.append(error)


    def vector_train_sgd(self, vector, target, time):
        # produce output
        self.forward_propagation(vector)
        # compute error
        error = self.layers[self.Nlayers-1].compute_error(target)
        # back propagate the error
        self.back_propagation(target)
        # update weights (gradient-descent+momentum)
        for i in range(1,self.Nlayers):
            self.layers[i].gradient_descent(self.learning_rate, \
            self.momentum, self.batchsize, self.weight_decay)
        #add error to error_list
        self.error_list.append(error)

    def batch_train_adam(self, batch, target, time):
        # batch size
        nbatch = len(batch)
        # error
        error = 0.0
        for n in range(nbatch):
            # produce output
            self.forward_propagation(batch[n])
            # compute error
            error += self.layers[self.Nlayers-1].compute_error(target[n])
            # back propagate the error
            self.back_propagation(target[n])
        # update weights (batch gradient-descent+momentum)
        for i in range(1,self.Nlayers):
            self.layers[i].adam(self.learning_rate, self.beta1, self.beta2,\
            self.epsilon, nbatch, self.weight_decay, time)
        #add error to error_list
        error /= nbatch
        self.error_list.append(error)


    def vector_train_adam(self, vector, target, time):
        # produce output
        self.forward_propagation(vector)
        # compute error
        error = self.layers[self.Nlayers-1].compute_error(target)
        # back propagate the error
        self.back_propagation(target)
        # update weights (gradient-descent+momentum)
        for i in range(1,self.Nlayers):
            self.layers[i].adam(self.learning_rate, self.beta1, self.beta2,\
            self.epsilon, nbatch, self.weight_decay, time)
        #add error to error_list
        self.error_list.append(error)


    def trainOnDataset(self, dataset, target):
        """
        Method that trains the network for classification
        on the given training dataset with corresponding target
        """
        # build target for classification if needed
        if self.scope=="Classification":
            nout = self.layers[self.Nlayers-1].n
            t = build_cl_target(target, nout)
        else:
            t = target
        # dimension of training dataset
        ntrain = len(dataset)
        # Sequential training
        if self.batchsize == 1:
            # select solver
            if self.solver == "sgd":
                update = self.vector_train_sgd
            elif self.solver == "adam":
                update = self.vector_train_adam
            time = 0
            for rounds in range(self.training_rounds):
                # shuffle dataset
                shuffled_dataset, shuffled_target = shuffleDT(dataset, t)
                for n in range(ntrain):
                    time += 1
                    update(shuffled_dataset[n],shuffled_target[n],time)

        # Batch training
        else:
            # select solver
            if self.solver == "sgd":
                update = self.batch_train_sgd
            elif self.solver == "adam":
                update = self.batch_train_adam
            # number of batches
            if self.batchsize>ntrain:
                self.batchsize = ntrain
                Nbatches = 1
            else:
                Nbatches = int(np.rint(ntrain/self.batchsize))
            # start training
            time = 0
            for rounds in range(self.training_rounds):
                # shuffle dataset
                shuffled_dataset, shuffled_target = shuffleDT(dataset, t)
                # split dataset into batches
                newset = np.array_split(shuffled_dataset, Nbatches)
                newtarget = np.array_split(shuffled_target, Nbatches)
                for n in range(Nbatches):
                    time += 1
                    update(newset[n],newtarget[n],time)

        if self.return_error:
            return self.error_list


    def validate(self, dataset, target):
        """
        Method that tests the network on the given
        validation dataset for classification problems
        and return the score
        """
        # dimension of validation dataset
        nvalidation = len(dataset)
        # init score
        score = 0.0
        # validate
        for n in range(nvalidation):
            self.forward_propagation(dataset[n])
            output = self.layers[self.Nlayers-1].pick_class()
            #output = self.classify(dataset[n])
            if output == target[n]:
                score = score + 1.0
        score = score/nvalidation
        return score


    def RMSE(self, dataset, target):
        """
        Method that computes and returns
        the network RMSE on the given dataset
        """
        # build target for classification if needed
        if self.scope=="Classification":
            nout = self.layers[self.Nlayers-1].n
            t = build_cl_target(target, nout)
        else:
            t = target
        # dimension of validation dataset
        nvalidation = len(dataset)
        # init score
        error = 0.0
        # validate
        for n in range(nvalidation):
            self.forward_propagation(dataset[n])
            # compute error
            error += self.layers[self.Nlayers-1].compute_error(t[n])
        error = m.sqrt(error/nvalidation)
        return error


    def trainAndValidate(self, dataset, target, fraction):
        """
        Method that trains and tests the network on
        the given dataset with corresponding target
        """
        if fraction>1.0 or fraction<0.0:
            print("ERROR: fraction must be a float between 0 and 1!")
            exit(1)
        # build traing and validation sets from dataset
        training_set, training_target, validation_set, validation_target = \
        shuffleAndSplitDT(dataset, target, fraction)
        # train
        error_list = self.trainOnDataset(training_set, training_target)
        # score
        training_score = self.validate(training_set, training_target)
        validation_score = self.validate(validation_set, validation_target)
        if self.return_error:
            return error_list, training_score, validation_score
        return training_score, validation_score


    def cross_validation(self, dataset, target, N):
        """
        Method that computes the accuracy of the
        network with the cross-validation procedure
        """
        fraction = 1.0-1.0/N
        score = 0.0
        for i in range(N):
            # build traing and validation sets from dataset
            training_set, training_target, validation_set, validation_target = \
            shuffleAndSplitDT(dataset, target, fraction)
            # train
            self.trainOnDataset(training_set, training_target)
            # add score
            score += self.validate(validation_set, validation_target)
        # compute average score
        score = score/N
        return score


    def activate(self, input_vector):
        """
        Method that returns the network output
        for the given input vector
        """
        self.forward_propagation(input_vector)
        output = self.layers[self.Nlayers-1].units
        return output


    def predict_dataset(self, dataset):
        """
        Method that returns the network output
        for the given dataset
        """
        # dimension of  dataset
        ndataset = len(dataset)
        # list for output
        output_list = []
        # produce output
        for n in range(ndataset):
            output = self.activate(dataset[n])
            output_list.append([])
            for i in range(len(output)):
                output_list[n].append(output[i])
        if len(output)>1:
            output_list = np.array(output_list)
            return output_list
        else:
            new_list = np.zeros(ndataset)
            for n in range(ndataset):
                new_list[n] = output_list[n][0]
            return new_list


    def classify(self, input_vector):
        """
        Method that returns the label  indicating
        the class with the highest probability
        for the given input vector.
        Must be used only for classification
        """
        self.forward_propagation(input_vector)
        output = self.layers[self.Nlayers-1].pick_class()
        return int(output)


    def classify_dataset(self, dataset):
        """
        Method that returns a list of labels
        indicating the classes of the input
        vectors in the given dataset.
        Must be used only for classification
        """
        # list for output
        output_list = []
        # produce output
        for element in dataset:
            output = self.classify(element)
            output_list.append(output)
        return np.array(output_list)


    def save(self, file_name):
        """
        Method that saves the network on a file named
        file_name
        """
        file_out = open(file_name,'w')
        # write network scope
        file_out.write("* Scope of network:\n")
        file_out.write(self.scope+"\n")
        # write layers dimensions
        file_out.write("* Dimension of layers:\n")
        for i in range(self.Nlayers):
            file_out.write(str(self.layers[i].n)+" ")
        file_out.write("\n")
        # write layers' types
        file_out.write("* Types of layers:\n")
        for i in range(self.Nlayers):
            file_out.write(self.layers[i].type+" ")
        file_out.write("\n")
        # write layers' biases
        for i in range(1,self.Nlayers):
            file_out.write("* Biases of layer "+str(i)+":\n")
            for j in range(0,self.layers[i].n):
                if self.layers[i].type=="Linear":
                    file_out.write(str(self.layers[i].biases[j])+" ")
                else:
                    file_out.write(str(self.layers[i].lin.biases[j])+" ")
            file_out.write("\n")
        # write layers' weights
        for i in range(1,self.Nlayers):
            file_out.write("* Weights of layer "+str(i)+":\n")
            for j in range(0,self.layers[i].n):
                for k in range(0,self.layers[i-1].n):
                    if self.layers[i].type=="Linear":
                        file_out.write(str(self.layers[i].weights[j][k])+" ")
                    else:
                        file_out.write(str(self.layers[i].lin.weights[j][k])+" ")
                file_out.write("\n")
        file_out.close()

    def save2(self, file_name):
        """
        Method that saves the network on a file named
        file_name
        *** Only to be used by C code ****
        """
        file_out = open(file_name,'w')
        # write number of layers
        file_out.write(str(self.Nlayers)+"\n")
        # write layers dimensions
        for i in range(self.Nlayers):
            file_out.write(str(self.layers[i].n)+" ")
        file_out.write("\n")
        # write layers' types
        for i in range(self.Nlayers):
            if self.layers[i].type == "Layer":
                file_out.write("input ")
            elif self.layers[i].type == "Linear":
                file_out.write("linear ")
            elif self.layers[i].type == "Tanh":
                file_out.write("tanh ")
            elif self.layers[i].type == "Sigmoid":
                file_out.write("sigmoid ")
            elif self.layers[i].type == "ReLU":
                file_out.write("relu ")
            elif self.layers[i].type == "MCTSL":
                file_out.write("mcts ")
            else:
                file_out.write("unknown ")
                print ("Warning: could not write type of layer "+str(i)+"\n")
        file_out.write("\n")
        # write layers' biases
        for i in range(1,self.Nlayers):
            for j in range(0,self.layers[i].n):
                if self.layers[i].type=="Linear":
                    file_out.write(str(self.layers[i].biases[j])+" ")
                else:
                    file_out.write(str(self.layers[i].lin.biases[j])+" ")
            file_out.write("\n")
        # write layers' weights
        for i in range(1,self.Nlayers):
            for j in range(0,self.layers[i].n):
                for k in range(0,self.layers[i-1].n):
                    if self.layers[i].type=="Linear":
                        file_out.write(str(self.layers[i].weights[j][k])+" ")
                    else:
                        file_out.write(str(self.layers[i].lin.weights[j][k])+" ")
                file_out.write("\n")
        file_out.close()


    def load(self, file_name):
        """
        Method that loads a network saved
        on a file named file_name
        """
        file_in = open(file_name,'r')
        # read network scope
        line = file_in.readline()
        line = file_in.readline()
        scope = line.split()
        self.scope = scope[0]
        # read layers' dimensions
        line = file_in.readline()
        line = file_in.readline()
        d = [int(n) for n in line.split()]
        # read layers' types
        line = file_in.readline()
        line = file_in.readline()
        types = line.split()
        # add layers to network
        self.add_layer("Input",d[0])
        for i in range(1,len(d)):
            self.add_layer(types[i],d[i])
        # read biases
        for i in range(1,len(d)):
            line = file_in.readline()
            line = file_in.readline()
            biases = [float(n) for n in line.split()]
            biases = np.array(biases)
            self.layers[i].set_biases(biases)
        # read weights
        for i in range(1,len(d)):
            line = file_in.readline()
            weights = []
            for j in range(self.layers[i].n):
                line = file_in.readline()
                weights.append([float(n) for n in line.split()])
            weights = np.array(weights)
            self.layers[i].set_weights(weights)
        # close input file
        file_in.close()

def build_cl_target(target, nout):
    """
    Function to build target of probabilities from
    class labels -- for classification networks only
    """
    ndata = len(target)
    t = np.zeros((ndata,nout))
    for i in range(ndata):
        t[i][target[i]] = 1.0
    return t


def shuffleDT(dataset, target):
    """
    Function to randomly shuffle a dataset
    and corresponding target
    """
    ntrain = len(dataset)
    shuffled_dataset = []
    shuffled_target = []
    random_index = np.random.permutation(ntrain)
    for n in range(ntrain):
        k = random_index[n]
        shuffled_dataset.append(dataset[k])
        shuffled_target.append(target[k])
    shuffled_dataset = np.array(shuffled_dataset)
    shuffled_target = np.array(shuffled_target)
    return shuffled_dataset, shuffled_target

def shuffleAndSplitDT(dataset, target, fraction):
    """
    Function to randomly shuffle a dataset
    and corresponding target and split them
    into a training and a validation set
    """
    ndata = len(dataset)
    shuffled_dataset, shuffled_target = shuffleDT(dataset, target)
    ntrain = int(ndata*fraction)
    training_set = []
    training_target = []
    validation_set = []
    validation_target = []
    for i in range(ntrain):
        training_set.append(shuffled_dataset[i])
        training_target.append(shuffled_target[i])
    for i in range(ntrain, ndata):
        validation_set.append(shuffled_dataset[i])
        validation_target.append(shuffled_target[i])
    training_set = np.array(training_set)
    training_target = np.array(training_target)
    validation_set = np.array(validation_set)
    validation_target = np.array(validation_target)
    return training_set, training_target, validation_set, validation_target
