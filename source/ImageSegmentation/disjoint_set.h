#pragma once


class Forest {
public:
	Forest(int treeCount);
	~Forest();
	int findRoot(int tree);
	void uniteRoots(int root1, int root2);
	int getSizeFromRoot(int root);
private:
	int *parent;
	int treeCount;

};

Forest::Forest(int treeCount) {
	this->treeCount = treeCount;
	parent = new int[treeCount];
	for (int i = 0; i < treeCount; i++) {
		parent[i] = -1;
	}
}


Forest::~Forest() {
	delete[] parent;
}


int Forest::findRoot(int tree) {
	int i = tree;
	if (parent[i] >= 0) {
		parent[i] = findRoot(parent[i]);
		return parent[i];
	}
	return i;
}

void Forest::uniteRoots(int r1, int r2) {
	if (parent[r1] < parent[r2]) {
		parent[r1] += parent[r2];
		parent[r2] = r1;		
	}
	else {
		parent[r2] += parent[r1];
		parent[r1] = r2;
	}
	return;
}

int Forest::getSizeFromRoot(int root) {
	return -parent[root];
}