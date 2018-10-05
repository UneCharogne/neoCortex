import numpy as np
import math as m
import random

class Layer (object):
    """
    A class to represent a general layer
    """
    def __init__(self, n):
        self.n = n
        self.units = np.zeros(n)
        self.type = "Layer"

    def activate(self, inputs):
        """
        Activate layer units
        """
        self.units = inputs


class Linear (Layer):
    """
    A class to represent a linear layer
    """
    def __init__(self, n, nprevious):
        Layer.__init__(self, n)
        self.nprevious = nprevious
        self.weights = np.zeros((n, nprevious))
        self.biases = np.zeros(n)
        self.grad_weights = np.zeros((n, nprevious))
        self.mom1_weights = np.zeros((n, nprevious))
        self.mom2_weights = np.zeros((n, nprevious))
        self.delta_weights = np.zeros((n, nprevious))
        self.delta_weights_old = np.zeros((n, nprevious))
        self.grad_biases = np.zeros(n)
        self.mom1_biases = np.zeros(n)
        self.mom2_biases = np.zeros(n)
        self.delta_biases = np.zeros(n)
        self.delta_biases_old = np.zeros(n)
        self.deltas = np.zeros(n)
        self.type = "Linear"

    def xavier_init_weights(self):
        """
        Initialize weights with Xavier initialization
        """
        mean = 0.0
        sigma = m.sqrt(2.0/(self.n+self.nprevious))
        for i in range(self.n):
            for j in range(self.nprevious):
                self.weights[i,j] = random.gauss(mean, sigma)

    def set_biases(self, biases):
        """
        Set biases of linear layer equal to array biases
        """
        self.biases = biases

    def set_weights(self, weights):
        """
        Set weights of linear layer equal to array weights
        """
        self.weights = weights

    def activate(self, previous_layer):
        """
        Activate layer units
        """
        self.units = np.dot(self.weights, previous_layer.units)+self.biases


    def delta_out(self, target):
        """
        Compute deltas when the layer is the output layer
        and the sum of square error function is used
        """
        self.deltas = self.units - target

    def delta(self, next_layer):
        """
        Compute deltas when the layer is an hidden layer
        """
        self.deltas = np.dot(next_layer.weights.transpose(),next_layer.deltas)

    def update_gradients(self, previous_layer):
        """
        Update delta wheights and biases
        """
        # delta biases
        self.grad_biases += self.deltas
        # delta weights
        self.grad_weights += np.outer(self.deltas,previous_layer.units)

    def update_moments(self, beta1, beta2, weight_decay):
        """
        Update moments of the gradients
        """
        # add weight decay term to gradients
        self.grad_weights += weight_decay*self.weights
        # first moment
        self.mom1_weights = beta1*self.mom1_weights+(1.0-beta1)*self.grad_weights
        self.mom1_biases = beta1*self.mom1_biases+(1.0-beta1)*self.grad_biases
        # second moment
        self.mom2_weights = beta2*self.mom2_weights+(1.0-beta2)*np.square(self.grad_weights)
        self.mom2_biases = beta2*self.mom2_biases+(1.0-beta2)*np.square(self.grad_biases)

    def adam(self, learning_rate, beta1, beta2, epsilon, batchsize, weight_decay, time):
        """
        Update wheights and biases with adam
        """
        # save old weights and biases
        self.weights_old = np.copy(self.weights)
        self.biases_old = np.copy(self.biases)
        # update moments of the gradients
        self.update_moments(beta1, beta2, weight_decay)
        # effective rate (for bias corrected moment estimates)
        effective_rate = learning_rate*m.sqrt(1.0-m.pow(beta2,time))/(1.0-m.pow(beta1,time))
        # update weights and biases
        self.delta_weights = -effective_rate*self.mom1_weights/\
        (np.sqrt(self.mom2_weights)+epsilon)
        self.weights = self.weights + self.delta_weights
        self.delta_biases = -effective_rate*self.mom1_biases/\
        (np.sqrt(self.mom2_biases)+epsilon)
        self.biases = self.biases + self.delta_biases
        # setting gradients to zero
        self.grad_weights.fill(0.0)
        self.grad_biases.fill(0.0)


    def gradient_descent(self, learning_rate, momentum, batchsize, weight_decay):
        """
        Update wheights and biases with stocastic gradient descent (with momentum)
        """
        # save old weights and biases
        self.weights_old = np.copy(self.weights)
        self.biases_old = np.copy(self.biases)
        # update weights and biases
        self.delta_weights = -learning_rate*(self.grad_weights/batchsize+\
        weight_decay*self.weights)+momentum*self.delta_weights_old
        self.weights = self.weights + self.delta_weights
        self.delta_biases = -learning_rate*self.grad_biases/batchsize+momentum*self.delta_biases_old
        self.biases = self.biases + self.delta_biases
        # saving old updates
        self.delta_weights_old = self.weights-self.weights_old
        self.delta_biases_old = self.biases-self.biases_old
        # setting gradients to zero
        self.grad_weights.fill(0.0)
        self.grad_biases.fill(0.0)

    def pick_class(self):
        """
        Return class with highest probability
        """
        return np.argmax(self.units)

    def compute_error(self, target):
        """
        Return error (sum-of-squares)
        """
        error_list = 0.5*(target-self.units)*(target-self.units)
        error = np.sum(error_list)
        return error


class Softmax (Layer):
    """
    A class to represent an element-wise layer
    with Softmax activation function
    """
    def __init__(self, n, nprevious):
        if n>1:
            Layer.__init__(self, n)
            self.lin = Linear(n, nprevious)
            self.type = "Softmax"
        else:
            print "ERROR: Softmax layer cannot have dimension 1!"
            exit(1)

    def xavier_init_weights(self):
        """
        Initialize weights with Xavier initialization
        """
        self.lin.xavier_init_weights()

    def set_biases(self, biases):
        """
        Set biases of linear layer equal to array biases
        """
        self.lin.set_biases(biases)

    def set_weights(self, weights):
        """
        Set weights of linear layer equal to array weights
        """
        self.lin.set_weights(weights)

    def activate(self, previous_layer):
        """
        Activate layer units
        """
        # activate linear layer
        self.lin.activate(previous_layer)
        # activate tanh layer
        self.units = np.exp(self.lin.units)
        norm = np.sum(self.units)
        self.units = self.units/norm

    def delta_out(self, target):
        """
        Compute deltas when the layer is the output layer
        and the cross-entropy error function is used
        """
        self.lin.deltas = self.units - target

    def update_gradients(self, previous_layer):
        """
        Update delta wheights and biases
        """
        self.lin.update_gradients(previous_layer)

    def adam(self, learning_rate, beta1, beta2, epsilon, batchsize, weight_decay, time):
        """
        Update wheights and biases with adam
        """
        self.lin.adam(learning_rate, beta1, beta2, epsilon, batchsize, weight_decay, time)

    def gradient_descent(self, learning_rate, momentum, batchsize, weight_decay):
        """
        Update wheights and biases with batch gradient descent (with momentum term)
        """
        self.lin.gradient_descent(learning_rate, momentum, batchsize, weight_decay)

    def pick_class(self):
        """
        Return class with highest probability
        """
        return np.argmax(self.units)


    def compute_error(self, target):
        """
        Return error (cross-entropy)
        """
        error_list = -target*np.log(self.units)
        error = np.sum(error_list)
        return error


class Tanh (Layer):
    """
    A class to represent an element-wise layer
    with hyperbolic tangent activation function
    """
    def __init__(self, n, nprevious):
        Layer.__init__(self, n)
        self.lin = Linear(n, nprevious)
        self.type = "Tanh"

    def xavier_init_weights(self):
        """
        Initialize weights with Xavier initialization
        """
        self.lin.xavier_init_weights()

    def set_biases(self, biases):
        """
        Set biases of linear layer equal to array biases
        """
        self.lin.set_biases(biases)

    def set_weights(self, weights):
        """
        Set weights of linear layer equal to array weights
        """
        self.lin.set_weights(weights)

    def activate(self, previous_layer):
        """
        Activate layer units
        """
        # activate linear layer
        self.lin.activate(previous_layer)
        # activate tanh layer
        self.units = np.tanh(self.lin.units)

    def delta(self, next_layer):
        """
        Compute deltas when the layer is an hidden layer
        """
        tanhprime = 1.0/(np.power(np.cosh(self.lin.units),2))
        self.lin.deltas = tanhprime*np.dot(next_layer.weights.transpose(),next_layer.deltas)

    def delta_out(self, target):
        """
        Compute deltas when the layer is the output layer
        and the sum-of-square error function is used
        """
        tanhprime = 1.0/(np.power(np.cosh(self.lin.units),2))
        self.lin.deltas = tanhprime*(self.units - target)


    def update_gradients(self, previous_layer):
        """
        Update delta wheights and biases
        """
        self.lin.update_gradients(previous_layer)

    def adam(self, learning_rate, beta1, beta2, epsilon, batchsize, weight_decay, time):
        """
        Update wheights and biases with adam
        """
        self.lin.adam(learning_rate, beta1, beta2, epsilon, batchsize, weight_decay, time)

    def gradient_descent(self, learning_rate, momentum, batchsize, weight_decay):
        """
        Update wheights and biases with batch gradient descent (with momentum term)
        """
        self.lin.gradient_descent(learning_rate, momentum, batchsize, weight_decay)

    def pick_class(self):
        """
        Return class with highest probability
        """
        if self.n>1:
            return np.argmax(self.units)
        else:
            if self.units[0]>0.0:
                return 1
            else:
                return 0

    def compute_error(self, target):
        """
        Return error (sum-of-squares)
        """
        error_list = 0.5*(target-self.units)*(target-self.units)
        error = np.sum(error_list)
        return error


class Sigmoid (Layer):
    """
    A class to represent an element-wise layer
    with sigmoid activation function
    """
    def __init__(self, n, nprevious):
        Layer.__init__(self, n)
        self.lin = Linear(n, nprevious)
        self.type = "Sigmoid"

    def xavier_init_weights(self):
        """
        Initialize weights with Xavier initialization
        """
        self.lin.xavier_init_weights()

    def set_biases(self, biases):
        """
        Set biases of linear layer equal to array biases
        """
        self.lin.set_biases(biases)

    def set_weights(self, weights):
        """
        Set weights of linear layer equal to array weights
        """
        self.lin.set_weights(weights)

    def activate(self, previous_layer):
        """
        Activate layer units
        """
        # activate linear layer
        self.lin.activate(previous_layer)
        # activate sigmoid layer
        self.units = 1.0/(1.0+np.exp(-self.lin.units))

    def delta(self, next_layer):
        """
        Compute deltas when the layer is an hidden layer
        """
        sigmoidprime = self.units*(1.0-self.units)
        self.lin.deltas = sigmoidprime*np.dot(next_layer.weights.transpose(),next_layer.deltas)

    def delta_out(self, target):
        """
        Compute deltas when the layer is the output layer
        and the sum-of-squares error function is used
        """
        self.lin.deltas = self.units*(1.0-self.units)*(self.units-target)

    def update_gradients(self, previous_layer):
        """
        Update delta wheights and biases
        """
        self.lin.update_gradients(previous_layer)

    def adam(self, learning_rate, beta1, beta2, epsilon, batchsize, weight_decay, time):
        """
        Update wheights and biases with adam
        """
        self.lin.adam(learning_rate, beta1, beta2, epsilon, batchsize, weight_decay, time)

    def gradient_descent(self, learning_rate, momentum, batchsize, weight_decay):
        """
        Update wheights and biases with batch gradient descent (with momentum term)
        """
        self.lin.gradient_descent(learning_rate, momentum, batchsize, weight_decay)

    def pick_class(self):
        """
        Return class with highest probability
        """
        if self.n>1:
            return np.argmax(self.units)
        else:
            return np.rint(self.units[0])


    def compute_error(self, target):
        """
        Return error (sum-of-squares)
        """
        error_list = 0.5*(target-self.units)*(target-self.units)
        error = np.sum(error_list)
        return error


class SoftSign (Layer):
    """
    A class to represent an element-wise layer
    with SoftSign activation function
    """
    def __init__(self, n, nprevious):
        Layer.__init__(self, n)
        self.lin = Linear(n, nprevious)
        self.type = "SoftSign"

    def xavier_init_weights(self):
        """
        Initialize weights with Xavier initialization
        """
        self.lin.xavier_init_weights()

    def set_biases(self, biases):
        """
        Set biases of linear layer equal to array biases
        """
        self.lin.set_biases(biases)

    def set_weights(self, weights):
        """
        Set weights of linear layer equal to array weights
        """
        self.lin.set_weights(weights)

    def activate(self, previous_layer):
        """
        Activate layer units
        """
        # activate linear layer
        self.lin.activate(previous_layer)
        # activate softsign layer
        self.units = self.lin.units/(1.0+np.absolute(self.lin.units))

    def delta(self, next_layer):
        """
        Compute deltas when the layer is an hidden layer
        """
        softsignprime = 1.0/np.power(1.0+np.absolute(self.lin.units),2)
        self.lin.deltas = softsignprime*np.dot(next_layer.weights.transpose(),next_layer.deltas)

    def delta_out(self, target):
        """
        Compute deltas when the layer is the output layer
        and the sum-of-squares error function is used
        """
        softsignprime = 1.0/np.power(1.0+np.absolute(self.lin.units),2)
        self.lin.deltas = softsignprime*(self.units - target)

    def update_gradients(self, previous_layer):
        """
        Update delta wheights and biases
        """
        self.lin.update_gradients(previous_layer)

    def adam(self, learning_rate, beta1, beta2, epsilon, batchsize, weight_decay, time):
        """
        Update wheights and biases with adam
        """
        self.lin.adam(learning_rate, beta1, beta2, epsilon, batchsize, weight_decay, time)

    def gradient_descent(self, learning_rate, momentum, batchsize, weight_decay):
        """
        Update wheights and biases with batch gradient descent (with momentum term)
        """
        self.lin.gradient_descent(learning_rate, momentum, batchsize, weight_decay)

    def pick_class(self):
        """
        Return class with highest probability
        """
        if self.n>1:
            return np.argmax(self.units)
        else:
            if self.units[0]>0.0:
                return 1
            else:
                return 0

    def compute_error(self, target):
        """
        Return error (sum-of-squares)
        """
        error_list = 0.5*(target-self.units)*(target-self.units)
        error = np.sum(error_list)
        return error


class ReLU (Layer):
    """
    A class to represent an element-wise layer
    with ReLU activation function
    """
    def __init__(self, n, nprevious):
        Layer.__init__(self, n)
        self.lin = Linear(n, nprevious)
        self.type = "ReLU"

    def xavier_init_weights(self):
        """
        Initialize weights with Xavier initialization
        """
        self.lin.xavier_init_weights()

    def set_biases(self, biases):
        """
        Set biases of linear layer equal to array biases
        """
        self.lin.set_biases(biases)

    def set_weights(self, weights):
        """
        Set weights of linear layer equal to array weights
        """
        self.lin.set_weights(weights)

    def activate(self, previous_layer):
        """
        Activate layer units
        """
        # activate linear layer
        self.lin.activate(previous_layer)
        # activate relu layer
        self.units = self.lin.units*(self.lin.units>0)

    def delta(self, next_layer):
        """
        Compute deltas when the layer is an hidden layer
        """
        reluprime = 1.0*(self.lin.units>0)
        self.lin.deltas = reluprime*np.dot(next_layer.weights.transpose(),next_layer.deltas)

    def delta_out(self, target):
        """
        Compute deltas when the layer is the output layer
        and the sum-of-squares error function is used
        """
        reluprime = 1.0*(self.lin.units>0)
        self.lin.deltas = reluprime*(self.units - target)

    def update_gradients(self, previous_layer):
        """
        Update delta wheights and biases
        """
        self.lin.update_gradients(previous_layer)

    def adam(self, learning_rate, beta1, beta2, epsilon, batchsize, weight_decay, time):
        """
        Update wheights and biases with adam
        """
        self.lin.adam(learning_rate, beta1, beta2, epsilon, batchsize, weight_decay, time)

    def gradient_descent(self, learning_rate, momentum, batchsize, weight_decay):
        """
        Update wheights and biases with batch gradient descent (with momentum term)
        """
        self.lin.gradient_descent(learning_rate, momentum, batchsize, weight_decay)

    def pick_class(self):
        """
        Return class with highest probability
        """
        if self.n>1:
            return np.argmax(self.units)
        else:
            if self.units[0]>0.0:
                return 1
            else:
                return 0

    def compute_error(self, target):
        """
        Return error (sum-of-squares)
        """
        error_list = 0.5*(target-self.units)*(target-self.units)
        error = np.sum(error_list)
        return error

class Support(Layer):
    """
    A supprt class needed for MCTSL layer
    """
    def __init__(self, n, nprevious):
        Layer.__init__(self, n)
        self.nprevious = nprevious
        self.weights = np.zeros((n, nprevious))
        self.biases = np.zeros(n)
        self.deltas = np.zeros(n)

class MCTSL (Layer):
    """
    A class to represent an element-wise layer
    with ReLU activation function
    """
    def __init__(self, n, nprevious):
        Layer.__init__(self, n)
        self.p = Softmax(n-1, nprevious)
        self.v = Tanh(1, nprevious)
        self.lin = Support(n, nprevious)
        self.type = "MCTSL"

    def xavier_init_weights(self):
        """
        Initialize weights with Xavier initialization
        """
        self.p.xavier_init_weights()
        self.v.xavier_init_weights()
        self.update_params()

    def set_biases(self, biases):
        """
        Set biases of linear layer equal to array biases
        """
        self.lin.biases = biases
        self.p.set_biases(biases[0:self.n-1])
        self.v.set_biases(np.array([biases[self.n-1]]))

    def set_weights(self, weights):
        """
        Set weights of linear layer equal to array weights
        """
        self.lin.weights = weights
        self.p.set_weights(weights[0:self.n-1])
        self.v.set_weights(np.array([weights[self.n-1]]))


    def activate(self, previous_layer):
        """
        Activate layer units
        """
        self.p.activate(previous_layer)
        self.v.activate(previous_layer)

    def update_params(self):
        """
        Update weights and biases of the layer
        from its sub-layers
        """
        self.lin.weights = np.concatenate((self.p.lin.weights, self.v.lin.weights))
        self.lin.biases = np.concatenate((self.p.lin.biases, self.v.lin.biases))

    def delta_out(self, targetp, targetv):
        """
        Compute deltas when the layer is the output layer
        and the sum-of-squares error function is used
        """
        self.p.delta_out(targetp)
        self.v.delta_out(targetv)
        self.lin.deltas = np.concatenate((self.p.lin.deltas, self.v.lin.deltas))
        self.lin.weights = np.concatenate((self.p.lin.weights, self.v.lin.weights))
        self.lin.biases = np.concatenate((self.p.lin.biases, self.v.lin.biases))

    def update_gradients(self, previous_layer):
        """
        Update delta wheights and biases
        """
        self.p.update_gradients(previous_layer)
        self.v.update_gradients(previous_layer)

    def adam(self, learning_rate, beta1, beta2, epsilon, batchsize, weight_decay, time):
        """
        Update wheights and biases with adam
        """
        self.p.adam(learning_rate, beta1, beta2, epsilon, batchsize, weight_decay, time)
        self.v.adam(learning_rate, beta1, beta2, epsilon, batchsize, weight_decay, time)

    def gradient_descent(self, learning_rate, momentum, batchsize, weight_decay):
        """
        Update wheights and biases with batch gradient descent (with momentum term)
        """
        self.p.gradient_descent(learning_rate, momentum, batchsize, weight_decay)
        self.v.gradient_descent(learning_rate, momentum, batchsize, weight_decay)


    def compute_error(self, targetp, targetv):
        """
        Return error (sum-of-squares)
        """
        error = 0.0
        error += self.p.compute_error(targetp)
        error += self.v.compute_error(targetv)
        return error
