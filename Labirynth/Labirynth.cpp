#include "stdafx.h"
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include <iostream>
#include <string>
#include <list>
#include <queue>

using namespace cv;
using namespace std;

typedef struct Nod {
	Nod* parent;
	list<Nod*> neighbours;
	int fieldValue, distanceToGoal;
	unsigned int totalCost;
	Point position;
} Node;

// Image proccesing
bool getMap(Mat &source) {
	string s;
	cout << "Map file path: ";
	cin >> s;
	source = imread(s);
	if (source.empty()) {
		cout << "Could not open or find the image!\n";
		return false;
	}
	return true;
}

Mat drawPoint(Mat image, Point point, const Scalar color) {
	circle(image, point, 2, color, 2, 8, 0);
	return image;
}

Mat drawLine(Mat image, Point pointA, Point pointB) {
	line(image, pointA, pointB, CV_RGB(0, 255, 0), 1, 8, 0);
	return image;
}

int** getValue(Mat image, int** Value) {
	for (int i = 0; i < image.rows; i++) {
		for (int j = 0; j < image.cols; j++) {
			uchar* tmp_ptr = &((uchar*)(image.data + image.step*i))[j * 3];
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

int distance(Point pointA, Point pointB) {
	return (int)sqrt((pointA.x - pointB.x)*(pointA.x - pointB.x) + (pointA.y - pointB.y)*(pointA.y - pointB.y));
}

int distanceManhattan(Point pointA, Point pointB) {
	return abs(pointA.x - pointB.x) + abs(pointA.y - pointB.y);
}

Nod *newNode(int value, int distance) {
	Nod* tmp = new Nod;
	tmp->fieldValue = value;
	tmp->distanceToGoal = distance;
	tmp->totalCost = INT_MAX; // -10000;
	tmp->parent = NULL;
	return tmp;
}

void addEdge(Nod* u, Nod* v) {
	u->neighbours.push_back(v);
	v->neighbours.push_back(u);
}

Nod** createGraph(int ** Value, int height, int width, Point goal) {
	Nod **graph = new Nod*[height*width];

	// Create node with values
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			int x = i * height + j;
			Point point = Point(j, i);
			graph[x] = newNode(Value[i][j], distanceManhattan(point, goal));
			graph[x]->position = point;
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
	while (!Q.empty()){ // && u != Goal) { // can finish faster but probably not the best path
		u = Q.top();
		Q.pop();
		list<Nod*>::iterator it;
		for (it = u->neighbours.begin(); it != u->neighbours.end(); it++) {
			if ((*it)->totalCost > u->totalCost + (*it)->fieldValue) { 
				(*it)->totalCost = u->totalCost + (*it)->fieldValue;
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
bool inRange(Mat source, Point point) {
	return (point.x >= 0 && point.x < source.cols && point.y >= 0 && point.y < source.rows);
}

bool getStartGoal(Mat sourceImage, Point &start, Point &goal) {
	printf("Start coords [x y]: ");
	cin >> start.x >> start.y;
	printf("Goal coords [x y]: ");
	cin >> goal.x >> goal.y;

	return (inRange(sourceImage, start) && inRange(sourceImage, goal));
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

int main(int argc, char** argv)
{	
	Mat src, path;
	Point Start, Goal;
	
	// Get Map
	if (getMap(src)) {
		printf("Map size: %dx%d\n", src.cols, src.rows);
	} else {
		return -1;
	}

	int Height = src.rows;
	int Width = src.cols;

	if (getStartGoal(src, Start, Goal)) {}
	else {
		// Default start & goal points
		printf("Default points!\n");
		Start.x = 0; Start.y = 0;
		Goal.x = 399; Goal.y = 399;
	}
	
	// Create variables
	int** Value = new int*[Height];
	for (int i = 0; i < Height; i++)
		Value[i] = new int[Width];
	
	// Color values matrix
	Value = getValue(src, Value);
	printf("START Field Cost: %d\n", Value[Start.x][Start.y]);
	printf("GOAL Field Cost: %d\n", Value[Goal.x][Goal.y]);
	
	// Create Graph
	int N = Height*Width;
	Nod **graph = createGraph(Value, Height, Width, Goal);
	
	// Search the best path to all points on map [Dijkstra]
	int StartNode = Start.y*Width + Start.x; 
	int GoalNode = Goal.y*Width + Goal.x;
	dijkstra(graph,N,graph[StartNode],graph[GoalNode]);
	
	// Uncomment below to see path tracking
	// writePath(graph[GoalNode], src);

	path = drawPath(graph[GoalNode], src);

	namedWindow("Path", WINDOW_AUTOSIZE);
	imshow("Path", src);

	printf("\n\t* * *\nTotal Cost:\t%d\nDistance:\t%d\n\t* * *\n", graph[GoalNode]->totalCost, graph[StartNode]->distanceToGoal);

	// Start Goal points drawing
	path = drawPoint(src, Start, CV_RGB(0, 0, 255));
	imshow("Path", path);
	path = drawPoint(path, Goal, CV_RGB(255, 0, 0));
	imshow("Path", path);
	
	// Cleanup 
	for (int i = 0; i < src.rows; i++)
		delete[] Value[i];
	delete[] Value;
	delete[] graph;
	
	waitKey(0);
	return 0;
}