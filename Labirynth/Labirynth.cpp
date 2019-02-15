#include "stdafx.h"
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include <iostream>
#include <string>
#include <list>
#include <queue>

using namespace cv;
using namespace std;

// Global variables
Mat src, gray, result;
 
struct point {
	int x;
	int y;
};

struct edge {
	int u, v;
	int s;		// edge cost 
};

typedef struct Nod {
	Nod* parent;
	list<Nod*> neighbours;
	int val, dist;
	unsigned int totalCost;
	Point position;
} Node;

// Image proccesing
bool getMap(Mat &src) {
	string s;
	cin >> s;
	src = imread(s);
	if (src.empty()) {
		cout << "Could not open or find the image!\n";
		return false;
	}
	return true;
}

Mat drawPoint(Mat img, Point p, const Scalar &color) {
	result = img;
	circle(result, p, 3, color, 2, 8, 0);
	return result;
}

Mat drawLine(Mat img, Point a, Point b) {
	result = img;
	line(result, a, b, CV_RGB(0, 255, 0), 1, 8, 0);
	return result;
}

int** getValue(Mat img, int** Value) {
	for (int i = 0; i < img.rows; i++) {
		for (int j = 0; j < img.cols; j++) {
			uchar* tmp_ptr = &((uchar*)(img.data + img.step*i))[j * 3];
			if ((int)tmp_ptr[0] == 0)
				Value[i][j] = 1;
			else 
				Value[i][j] = (int)tmp_ptr[0];
		}
	}
	return Value;
}

// Labirynth path search
auto cmp = [](Nod *x, Nod *y) {
	return x->totalCost > y->totalCost;
};

bool operator< (Nod a, Nod b) {
	if (a.totalCost < b.totalCost)
		return true;
	else
		return false;
}

int distance(Point p1, Point p2) {
	return (int)sqrt((p1.x - p2.x)*(p1.x - p2.x) + (p1.y - p2.y)*(p1.y - p2.y));
}

Nod *newNode(int val, int dist) {
	Nod* tmp = new Nod;
	tmp->val = val;
	tmp->dist = dist;
	tmp->totalCost = INT_MAX; // -10000;
	tmp->parent = NULL;
	return tmp;
}

void addEdge(Nod* u, Nod* v) {
	u->neighbours.push_back(v);
	v->neighbours.push_back(u);
}

void writeList(list<Nod*> L) {
	while (!L.empty()) {
		Nod* node = L.front();
		L.pop_front();
		printf("[%d,%d] : %d ", node->position.x, node->position.y, node->totalCost);
	}
	printf("\n");
}

void writeQueue(priority_queue<Nod *> Q) {
	while (!Q.empty()) {
		Nod* tmpNode = Q.top();
		Q.pop();
		printf("[%d,%d] (%d); ", tmpNode->position.x, tmpNode->position.y, tmpNode->totalCost);
	}
}

Nod** createGraph(int ** Value, int height, int width, Point goal) {
	Nod **graph = new Nod*[height*width];

	// Create node with values
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			int x = i * height + j;
			Point p = Point(j, i);
			graph[x] = newNode(Value[i][j], distance(p, goal));
			graph[x]->position = p;
		}
	}

	// Create neighbours list
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			int x = i * height + j;
			if (x + 1 < (i + 1)*height) {
				addEdge(graph[x], graph[x + 1]);
			}
			if (x + width < height*width) {
				addEdge(graph[x], graph[x + width]);
			}
		}
	}

	return graph;
}

void dijkstra(Nod **graph, int N, Nod *Start, Nod *Goal) {
	priority_queue <Nod *, vector<Nod *>, decltype(cmp)> Q(cmp);

	Q.push(Start);
	Nod* u = Q.top();
	u->totalCost = 0;
	while (!Q.empty() && u != Goal) {
		u = Q.top();
		Q.pop();
		list<Nod*>::iterator it;
		for (it = u->neighbours.begin(); it != u->neighbours.end(); it++) {
			if ((*it)->totalCost > u->totalCost + (*it)->val) { 
				(*it)->totalCost = u->totalCost + (*it)->val;
				(*it)->parent = u;
				Q.push((*it));
			}
		}
	}
}

void writePath(Nod* Goal) {
	while (Goal->parent != NULL) {
		printf("(%d,%d) ", Goal->position.x, Goal->position.y);
		Goal = Goal->parent;
	}
}

Mat drawPath(Nod* Goal, Mat img) {
	while (Goal->parent != NULL) {
		img = drawLine(img, Goal->position, Goal->parent->position);
		Goal = Goal->parent;
	}
	return img;
}

int main(int argc, char** argv)
{	
	Mat src, gray, result;

	// Create window
	namedWindow("Map", WINDOW_AUTOSIZE);
	
	// Default start
	Point Start, Goal;
	Start.x = 0; Start.y = 0;
	Goal.x = 399; Goal.y = 399;

	// Get Map
	if (getMap(src))	
		imshow("Map", src);
	else  return -1;
	int Height = src.rows;
	int Width = src.cols;

	// Creare variables
	int** Value = new int*[Height];
	for (int i = 0; i < Height; i++)
		Value[i] = new int[Width];
	
	// Color values matrix
	Value = getValue(src, Value);
	printf("START_COLOR: %d\n", Value[Start.x][Start.y]);
	printf("GOAL_COLOR: %d\n", Value[Goal.x][Goal.y]);
	printf("Size: %d\n", src.size());
	
	// Create Graph
	int N = Height*Width;
	Nod **graph = createGraph(Value, Height, Width, Goal);
	
	int StartNode, GoalNode;
	StartNode = Start.y*Width + Start.x;
	GoalNode = Goal.y*Width + Goal.x;
	Nod *ss = graph[StartNode];
	Nod *gg = graph[GoalNode];
	dijkstra(graph,N,ss,gg);

	/*	// start debug 2
	printf("debug 2\n");
	Nod *tmpDbg = new Nod();
	list<Nod*>::iterator it;
	for (it = ss->neighbours.begin(); it != ss->neighbours.end(); it++) {
		printf(" -> [%d,%d], %d\n", (*it)->position.x, (*it)->position.y, (*it)->val);
		tmpDbg = (*it);
	}
	printf(" -> [%d,%d], %d\n", tmpDbg->parent->position.x, tmpDbg->parent->position.y, tmpDbg->parent->val);
	// end debug 2	*/
	namedWindow("Path", WINDOW_AUTOSIZE);
	//writePath(graph[GoalNode], src);
	result = drawPath(graph[GoalNode], src);
	//line(result, Point(1, 1), Point(50, 50), CV_RGB(255, 0, 0), 3, 8, 0);
	imshow("Path", src);
	/*	// start debug 3
	printf("debug 3\nGOAL\n");
	printf(" -> [%d,%d], %d\n", gg->position.x, gg->position.y, gg->val);
	if (gg->parent != NULL)
		printf(" -> [%d,%d], %d\n", gg->parent->position.x, gg->parent->position.y, gg->parent->val);
	else
		printf("There is no parent for [%d,%d]\n", gg->position.x, gg->position.y);
	// end debug 3	*/

	destroyWindow("Map");
	int xx = Goal.y*Height + Goal.x;
	printf("pkt: %d %d %d", graph[xx]->dist, graph[xx]->val, graph[xx]->totalCost);
	//Start Goal drawing
	result = drawPoint(src, Start, CV_RGB(0, 0, 255));
	imshow("Path", result);
	result = drawPoint(src, Goal, CV_RGB(255, 0, 0));
	imshow("Path", result);

	
	for (int i = 0; i < src.rows; i++)
		delete[] Value[i];
	delete[] Value;

	delete[] graph;
	waitKey(0);
	return 0;
}