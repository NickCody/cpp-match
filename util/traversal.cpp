#include <iostream>
#include <memory>
#include <iomanip>
using namespace std;

class Node;
using NodePtr = shared_ptr<Node>;

class Node {
public:
  Node(const string& id, NodePtr left = nullptr, NodePtr right = nullptr)
      : id(id),
        left(left),
        right(right) {
  }

  std::string id;
  NodePtr left;
  NodePtr right;
};

void print_node(NodePtr root, int indent = 0) {
  if (!root) {
    cout << std::setfill(' ') << std::setw(indent) << "-" << endl;
  } else {
    cout << std::setfill(' ') << std::setw(indent) << root->id << endl;
    print_node(root->left, indent + 2);
    print_node(root->right, indent + 2);
  }
}

void in_order_traversal(NodePtr node) {
  if (!node)
    return;

  in_order_traversal(node->left);
  cout << node->id << endl;
  in_order_traversal(node->right);
}

void pre_order_traversal(NodePtr node) {
  if (!node)
    return;

  cout << node->id << endl;
  in_order_traversal(node->left);
  in_order_traversal(node->right);
}

void post_order_traversal(NodePtr node) {
  if (!node)
    return;

  in_order_traversal(node->left);
  in_order_traversal(node->right);
  cout << node->id << endl;
}

int main() {

  NodePtr root =
      make_shared<Node>("D", make_shared<Node>("B", make_shared<Node>("A"), make_shared<Node>("C")), make_shared<Node>("F", make_shared<Node>("E")));

  print_node(root);

  cout << "in" << endl;
  in_order_traversal(root);
  cout << "pre" << endl;
  pre_order_traversal(root);
  cout << "post" << endl;
  post_order_traversal(root);

  return 0;
}