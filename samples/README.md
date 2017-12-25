## Samples
* heart-disease uses 13 features as input
* housing uses 13 features as input
* spambase uses 57 features as input
* the artificial model uses 16 features as input

## Input Format
### Tree model file
1. The first line indicates the thresholds used in the decision tree, seperated by comma.
2. The second line indicates the mapping from node index to the feature index. The mapping is an integer pair `(int:int)`, and pairs are seperated by comma.
3. Following lines indicate a path from the root node to a leaf node. The integer indicate the node index in the tree. The format of the path is `[int,int,....]`. Closed inside `[]`, and integers inside are seperated by comma.
4. The node and feature indices all start from 0.
