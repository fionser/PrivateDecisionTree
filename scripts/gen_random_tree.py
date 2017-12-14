import sys
import numpy as np
class TreeNode:
    def __init__(self, ID, weight):
        self.ID = ID
        self.weight = weight
        self.depth = 0
        self.left = None
        self.right = None
        
    def set_left(self, left_child):
        self.left = left_child

    def set_right(self, right_child):
        self.right = right_child

class Tree:
    def __init__(self, max_depth, num_features):
        self.ID = 0
        self.num_features = num_features
        self.max_depth = max_depth
        self.weights = []
        self.id_index_map = dict()
        self.root = self.create_node()
        self.leafs = [self.root]

    def create_node(self):
        w = np.random.randint(1, 1000)
        indx = np.random.randint(0, self.num_features)
        self.id_index_map[self.ID] = indx
        ID = self.ID
        self.ID = self.ID + 1
        self.weights.append('%d' % w)
        return TreeNode(ID, w)

    def extend(self):
        random_leaf = np.random.randint(len(self.leafs))
        leaf = self.leafs[random_leaf]
        #if leaf.depth + 1 > self.max_depth:
        #    return
        self.leafs = self.leafs[:random_leaf] + self.leafs[random_leaf + 1:]
        leaf.left = self.create_node()
        leaf.right = self.create_node()
        leaf.left.depth = leaf.depth + 1
        leaf.right.depth = leaf.depth + 1
        self.leafs.append(leaf.left)
        self.leafs.append(leaf.right)

    def print_tree(self):
        self.print_weight()
        self.print_map()
        self._print(self.root, "")

    def print_map(self):
        print(",".join(["%d:%d" % (i, f) for (i, f) in self.id_index_map.items()]))

    def print_weight(self):
        print(",".join(self.weights))

    def _print(self, tree_root, msg):
        if not tree_root:
            return
        if not tree_root.left and not tree_root.right:
            print('[%s,%d]' % (msg, tree_root.ID))
        else:
            if len(msg) > 0:
                msg = '%s,%d' % (msg, tree_root.ID)
            else:
                msg = '%d' % tree_root.ID
            self._print(tree_root.left, msg)
            self._print(tree_root.right, msg)

def random_bit_streams(sze):
    return [np.random.randint(0, 2) for _ in range(sze)]

# usage prog <num_of_internal_nodes> <max_depth> <num_of_features>
def main():
    if (len(sys.argv) != 4):
        print ("prog <num_of_internal_nodes> <max_depth> <num_of_features>")
        return
    cnt_internal_nodes = int(sys.argv[1])
    max_depth = int(sys.argv[2])
    num_features = int(sys.argv[3])
    total_nodes = 1 + (cnt_internal_nodes << 1)
    tr = Tree(max_depth, num_features) 
    while tr.ID < total_nodes:
        tr.extend()
    tr.print_tree()

if __name__ == "__main__":
    main()
