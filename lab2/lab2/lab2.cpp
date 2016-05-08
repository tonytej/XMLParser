// lab2.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <stack>
#include <deque>
#include <regex>

using namespace std;

/*! A class containing a vector of string that makes up a node */
class Node {
	private:
		vector<string> attributes;
	public:
		void insertAttribute(string att);
		friend bool operator<(const Node& lhs, const Node& rhs);
		void clearVector();
		string getAttributes(int subscript);
};

/**
* Insert element to the vector of string
*
* @param att string to be pushed
*/
void Node::insertAttribute(string att) {
	attributes.push_back(att);
}

void Node::clearVector() {
	attributes.clear();
}

/**
* Get attributes from the private vector
*
* @param subscript subscript to access specific attributes
* @return element inside attribute
*/
string Node::getAttributes(int subscript) {
	return attributes[subscript];
}

/*! A class containing regular expressions functions needed to parse the XML file */
class regexFunctions {
private:
	string pattern1;
public:
	bool search(string text, string expr);
	string split(string text, string split);
	string replace(string text, string find, string value);
};

/**
* Search for strings that match the expression
*
* @param text text to be searched at
* @param expr expression to match
* @return status true if found else false
*/
bool regexFunctions::search(string text, string expr){
	bool status;
	regex pattern(expr);
	if (regex_search(text.begin(), text.end(), pattern)) {
		status = true;
	}
	else {
		status = false;
	}
	return status;
}

/**
* Split string of matched strings using expression
*
* @param text text to be splitted
* @param split expression to match
* @return splitted string
*/
string regexFunctions::split(string text, string split){
	sregex_token_iterator end;
	regex pattern(split);
	for (sregex_token_iterator iter(text.begin(), text.end(), pattern); iter != end; ++iter){
		if ((*iter).length() > 0)
		{
			if ((static_cast<string>(*iter))[0] != 0x20)
				return *iter;
		}
	}
	return "Split Fail!";
}

/**
* Replace matched string with other string
*
* @param text text to be processed
* @param find expression to be matched
* @param value string to replace matched string
* @return remaining string
*/
string regexFunctions::replace(string text, string find, string value){
	regex pattern(find);
	string sresult = regex_replace(text, pattern, value);
	return sresult;
}

/*! A class with a purpose of parsing the XML file */
class XMLParser {
	private:
		deque<string> elements;
		stack<string> tag;
		vector<Node> node;
		Node temp;
	public:
		void process(regexFunctions rf, string filepath);
		void sortVector();
		vector<Node> getVector();
};

/**
* Process the XML file
*
* @param rf regexFunctions class providing necessary functions
* @param filepath string of the xml file path
*/
void XMLParser::process(regexFunctions rf, string filepath) {
	ifstream fin(filepath);
	if (fin.fail()) {
		cout << "Input file failed to open." << endl;
		exit(1);
	}
	string str;
	int count = 0;
	while (!fin.eof()) {
		getline(fin, str);
		if (rf.search(str, "\t</") || rf.search(str, " </")) { //termination condition
			string var = str;
			if (tag.top() == var) {
				tag.pop();
			}
			if (count == 5) {//if all data is parsed (end of one node)
				count = 0;//reset counter due to end of one node
				for (size_t i = 0; i < elements.size(); i++) {
					temp.insertAttribute(elements[i]);//insert each element from deque to node
				}
				node.push_back(temp);//insert node to a vector of node
				temp.clearVector();//clear node for next set of data
				elements.clear();//clear deque for next set of data
			}
		}
		else if (rf.search(str, ">(.*?)<")) { //elements condition
			string var = rf.split(str, ">(.*?)(?=<)");//get data without tags
			var = rf.replace(var, ">", "");
			elements.push_back(var);//push var to deque
			count++;
		}
		else { //tags condition
			tag.push(str);
		}
	}
}

void XMLParser::sortVector() {
	sort(node.begin(), node.end());
}

/**
* Get the private vector of nodes from the class
* @return node
*/
vector<Node> XMLParser::getVector() {
	return node;
}

/*! A class containing functions to calculate the distance between two airports */
class distanceCalculator {
	private:
		/// @brief The usual PI/180 constant  
		const double DEG_TO_RAD = 0.017453292519943295769236907684886;
		/// @brief Earth's quatratic mean radius for WGS-84  
		const double EARTH_RADIUS_IN_METERS = 6372797.560856;
	public:
		int rBinarySearch(vector<Node> vec, int first, int last, string key);
		double ArcInRadians(Node& from, Node& to);
		double DistanceInMeters(Node& from, Node& to);
};

/**
* Binary Search Algorithm
*
* @param vec vector to be searched at
* @param first left boundary
* @param last right boundary
* @param code airport code to be searched
* @return index of the code
*/
int distanceCalculator::rBinarySearch(vector<Node> vec, int first, int last, string code) {
	if (first <= last) {
		int mid = (first + last) / 2;  // compute mid point.
		if (code == vec[mid].getAttributes(0))
			return mid;   // found it.
		else if (code < vec[mid].getAttributes(0))
			// Call ourself for the lower part of the array
			return rBinarySearch(vec, first, mid - 1, code);
		else
			// Call ourself for the upper part of the array
			return rBinarySearch(vec, mid + 1, last, code);
	}
	return -(first + 1);    // failed to find key
}

/**
* Computes the arc, in radian, between two WGS-84 positions.
*
* @param from initial position
* @param to destination
* @return arc in radians
*/
double distanceCalculator::ArcInRadians(Node& from, Node& to) {
	double fromLat = stod(from.getAttributes(3));
	double fromLon = stod(from.getAttributes(4));
	double toLat = stod(to.getAttributes(3));
	double toLon = stod(to.getAttributes(4));
	double latitudeArc = (fromLat - toLat) * DEG_TO_RAD;
	double longitudeArc = (fromLon - toLon) * DEG_TO_RAD;
	double latitudeH = sin(latitudeArc * 0.5);
	latitudeH *= latitudeH;
	double lontitudeH = sin(longitudeArc * 0.5);
	lontitudeH *= lontitudeH;
	double tmp = cos(fromLat*DEG_TO_RAD) * cos(toLat*DEG_TO_RAD);
	return 2.0 * asin(sqrt(latitudeH + tmp*lontitudeH));
}

/**
* Computes the distance, in meters, between two WGS-84 positions. 
*
* @param from initial position
* @param to destination
* @return distance in meters
*/
double distanceCalculator::DistanceInMeters(Node& from, Node& to) {
	return EARTH_RADIUS_IN_METERS*ArcInRadians(from, to);
}

/**
* Enable debugging in Visual Studio
*/
void enableDebug(bool bvalue){
	if (!bvalue) return;
	int tmpFlag = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
	// Turn on leak-checking bit.
	tmpFlag |= _CRTDBG_LEAK_CHECK_DF;
	// Turn off CRT block checking bit.
	tmpFlag &= ~_CRTDBG_CHECK_CRT_DF;
	// Set flag to the new value.
	_CrtSetDbgFlag(tmpFlag);
}

int main()
{
	enableDebug(true);
	XMLParser test;
	regexFunctions rf;
	distanceCalculator calc;
	test.process(rf, "Airports.xml");
	test.sortVector();
	int airport1 = calc.rBinarySearch(test.getVector(), 0, test.getVector().size(), "GSO");
	int airport2 = calc.rBinarySearch(test.getVector(), 0, test.getVector().size(), "HTS");
	double result = calc.DistanceInMeters(test.getVector()[airport1], test.getVector()[airport2]);
	cout << "Distance from GSO to HTS is " << result << " meters"<< endl;
}

/**
* Overloaded < operator for sorting vector of objects
*/
bool operator<(const Node& lhs, const Node& rhs) {
	return lhs.attributes[0] < rhs.attributes[0];
}

