#include <iostream>
#include <vector>
#include <fstream>
#include "quadtree.hpp"
#include <chrono>

using namespace std;
using namespace chrono;

class Clock
{
public:
	duration<double> time_passed{0};
	//chrono segítségével a system clockhoz mért időpontokat tudok létrehozni
	time_point<system_clock> start, end;

	//SEGÍTSÉG MAGAMNAK:
	// NANOSEC. - MICROSEC. - MILLISEC. - SECONDS - MINUTES - HOURS
	void TimeinNanoSec()
	{
		time_passed = (end - start);
		//Megkapom a keresett időt, majd nanoszekundumokká váltom át.
		auto _nanosec = duration_cast<nanoseconds>(time_passed);
		cout << _nanosec.count() << " nanoseconds" << endl;
	};

	void TimeinMicroSec()
	{
		time_passed = (end - start);
		auto _microsec = duration_cast<microseconds>(time_passed);
		cout << _microsec.count() << " microseconds" << endl;
	};

	void TimeinMilliSec()
	{
		time_passed = (end - start);
		auto _sec = duration_cast<milliseconds>(time_passed);
		cout << _sec.count() << " milliseconds" << endl;
	};
};

int main()
{
    try
    {
    	///Quadtree feltöltése elemekkel az "init_points.txt" fájlból:
    	ifstream f;
    	f.open("init_points.txt");

    	if (!f.good())
    	{
    	     cerr << "Hiba, nincs meg a file" << endl;
    	     exit(1);
    	}

		Quadtree* MyQuadTree = new Quadtree(Point2D(0.f,0.f), Point2D(1000.f,1000.f));

		float x, y;
		while (f.good())
		{
			f >> x;
			f >> y;
			MyQuadTree->insert(Point2D(x,y));
		}

		//Quadtree pontjai és rectangle fájlba íratása
		ofstream rect_txt("rect.txt"); // bal felső x; bal felső y; szélesség(x tengely); magasság(y tengely)
		ofstream point_txt("point2d.txt"); // x koord; y koord
		ofstream knn_txt("knn.txt");

		MyQuadTree->rect(rect_txt);
		MyQuadTree->points(point_txt);

		///KNN tesztelése

		Clock* kNN_test = new Clock;
		//Itt add meg, hogy mely koordinátájú pontra fusson le a kNN:
		Point2D knn_observed_point{300.f, 300.f};
		//Itt add meg, hogy hány darab pontra szeretnéd futtatni a szomszédkeresést:
		unsigned int neighbor_num = 150;
		kNN_test->start = system_clock::now();
		vector <Point2D> nearest_points = MyQuadTree->kFind(knn_observed_point, neighbor_num);
		kNN_test->end = system_clock::now();

		cout << "kNN futásának időtartama " << neighbor_num << " szomszéd keresése esetén: " << endl;
		kNN_test->TimeinNanoSec();

		//knn kiiratása fájlba
		knn_txt << knn_observed_point; //overloaded operator
		for (auto n:nearest_points)
		{
			knn_txt << n;
		}

    }
    catch (const exception& e)
    {
        cout << "HIBA: " << e.what() << endl;
    }


    return 0;
}
