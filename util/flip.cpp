#include <iostream>
#include <memory>

using namespace std;

class Node;
using NodePtr = shared_ptr<Node>;

class Node {
public:
  using NodePtr = shared_ptr<Node>;

  Node(const string& id, NodePtr left = nullptr, NodePtr right = nullptr)
      : id(id),
        left(left),
        right(right) {
  }

  std::string id;
  NodePtr left;
  NodePtr right;
};

void print_node(Node::NodePtr root) {
  if (!root) {
    cout << "-";
  } else {
    cout << root->id << "(";
    print_node(root->left);
    cout << ",";
    print_node(root->right);
    cout << ")";
  }
}

void flip(NodePtr root) {
  if (!root)
    return;

  NodePtr temp = root->left;
  root->left = root->right;
  root->right = temp;

  flip(root->left);
  flip(root->right);
}

NodePtr immutable_flip(NodePtr root) {
  if (!root)
    return root;

  return make_shared<Node>(root->id, immutable_flip(root->right), immutable_flip(root->left));
}

int main() {

  NodePtr root =
      make_shared<Node>("A", make_shared<Node>("B", make_shared<Node>("D"), make_shared<Node>("E")), make_shared<Node>("C", make_shared<Node>("F")));

  print_node(root);
  cout << endl;
  print_node(immutable_flip(root));
  cout << endl;

  return 0;
}