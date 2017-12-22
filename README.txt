To execute the program: 
python pca.py input_file_name

**** INPUT ****
There is an input example called “configurations.dat” which contains 4000 configurations of 2D Ising model with 400 spins at 40 different temperatures, going from T=1.0 to T=4.9 with dT=0.1 (100 independent configurations per temperature). Each line contains a configuration x=(s1,s2,…,s400). Configurations are ordered per temperature from lowest to highest.
This file is read and stored as a 4000(number of configurations) X 400 (number of spins) matrix.

**** PCA ****
PCA consists of 4 steps:
-Step 1: compute the covariance matrix of the input matrix.
-Step 2: compute eigenvalues and eigenvectors (which are the principal components) of the covariance matrix.
-Step 3: order eigenvectors (principal components) according to their eigenvalues going from the highest to the lowest.
-Step 4: project the configurations on the most important principal components (the program does this for the first 3 principal components) 

**** OUTPUT ****
The program produces two outputs:

-“results.dat” has 4 columns. The 1st column is only an index n indicating the configuration. The remaining 3 columns are the projections of the nth configuration on the first 3 principal components, respectively. 

-“norm_eigen.dat” contains the normalized eigenvalues of the principal components ordered from highest to lowest. These quantities indicates how much of the variance is contained on each principal components (the higher the variance the more the information).

**** IMAGES ****
I also put images:

-“2PC.eps” is a 2d plot of the projections of the configurations on the first 2 principal components. Points are colored according to the temperature.

-“eigenvalues.eps” shows the normalized eigenvalues of the principal components. In the case of the Ising model only the first component is relevant.



