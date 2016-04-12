#ifndef BIN_TREE_HPP_INCLUDED
#define BIN_TREE_HPP_INCLUDED

#include <iostream>
#include "exceptions.hpp"
#include <fstream>
#include <set>			//set <Point2D> points;
#include <cmath> 		//abszolútérték (abs)
#include <queue> 		//priority RectsByPriority-hoz szükséges
#include <algorithm> 	//vector -- sort

using namespace std;

///--------------------------------------------- POINT2D ---------------------------------------------

struct Point2D
{
	float x; //float-okkal nincs adatvesztés ((Egy-egy rectangle node szétosztásánál (Split függvény) ))
	float y;

	//megadja, hogy milyen messze van az adott pont a kNN-be beadott ponttól.
	float ptopdistance{0.f}; // Point to Point Distance

	Point2D(float _x, float _y) : x(_x), y(_y) {}

	bool operator<(const Point2D & other) const {return x < other.x;}

	friend ostream & operator<<(ostream & os, const Point2D & p)
	{
		os << p.x << " " << p.y << endl;
		return os;
	}

	//Frissíti a Point2D ptopdistance mezőjét a knn-be beadott pontnak a függvényében (comparator megvalósítás miatt szükséges)
	void DistanceOfTwo(Point2D & that)
	{
		ptopdistance = sqrt((that.x - this->x)*(that.x - this->x) + (that.y - this->y)*(that.y - this->y));
	}

};

//PriorityPoints priority RectsByPriority komparátora (lásd vector<Point2D> kFind(Point2D a, unsigned int k)),
//mely a kNN-be beadott pont-tól való távolság alapján segít a pr. RectsByPriority-ba sorbarendezni az elemeket.
struct ComparePointsD
{
	bool operator()(const Point2D & p1, const Point2D & p2) const
	{
		return p1.ptopdistance < p2.ptopdistance;
	}
};



///--------------------------------------------- QUADTREE ---------------------------------------------

//Pointeres Quadtree megoldás, Rectangle "node"-okkal, a root node is meg van nevezve, hogy a
//bináris fákhoz hasonló algoritmusokat lehessen írni.
class Quadtree
{
	class Rectangle
	{
	public:
		unsigned int max_objects{20};

		// Upper.x == bal felső sarok x koordinátája; Upper.y == ugyanez y-ra
		Point2D Upper;
		// Size.x == x tengely menti szélesség; Size.y == y-os magasság
		Point2D Size;

		Rectangle* parent;
		Rectangle* nw{nullptr};
		Rectangle* ne{nullptr};
		Rectangle* sw{nullptr};
		Rectangle* se{nullptr};

		set <Point2D> points; //pontok tárolója.
		//kiszűri az ismétlődéseket; nem insertelhetünk két ugyanolyan koordinátájú pontot.

		float point_distance{0.f};

		Rectangle(Point2D _Upper, Point2D _Size, Rectangle* _parent) : Upper(_Upper), Size(_Size), parent(_parent) {}

		bool IsLeaf() { return this->nw == nullptr; }

		//Ebben a rectangle-ben megtalálható az 'a' pont?
		bool Contains (Point2D a) const
		{
			if (Upper.x <= a.x && Upper.y <= a.y && (Upper.x + Size.x) >= a.x && (Upper.y + Size.y) >= a.y)
			{
				return true;
			}
			return false;
		}

		//Frissíti a Rectangle point_distance mezőjét a knn-be beadott pontnak a függvényében (comparator megvalósítás miatt szükséges)
		void DistToPoint(Point2D a)
		{

			//Lehetséges esetek:
			//(őszintén remélem a kommentelt "ábra" ( bár annak nem igazán mondható :D ) nem csúszott el
				  /**				*
			3.(nw)	* 1.(nw<a.x<ne) *	5.(ne)
					*	   a.y<		*
		*-*-*-*-*-*-*---------------*-*-*-*-*-*-*
			a.y<	|				|	a.y>
			2.		|	rectangle	|	2.
		(nw<a.y<ne)	|				|(nw<a.y<ne)
		*-*-*-*-*-*-*---------------*-*-*-*-*-*-*
					*				*
			4.(sw)	* 1.(nw<a.x<ne)	*	6.(se)
					*	  a.y>		**/


			// 1. eset(ek)
			if (a.x > this->Upper.x && a.x < this->Upper.x + this->Size.x)
			{
				point_distance = abs(a.y -  this->Upper.y);
			}
			// 2.eset(ek)
			else if (a.y >  this->Upper.y && a.y <  this->Size.y + this->Upper.y)
			{
				point_distance = abs(a.x - this->Upper.x);
			}
			//3.
			else if(a.x < this->Upper.x && a.y < this->Upper.y)
			{
				a.DistanceOfTwo(this->Upper);
				point_distance = a.ptopdistance;
			}
			//4.
			else if(a.x < this->Upper.x && a.y > this->Upper.y)
			{
				Point2D swcorner{Upper.x, Upper.y + Size.y};
				a.DistanceOfTwo(swcorner);
				point_distance = a.ptopdistance;
			}
			//5.
			else if(a.x > this->Upper.x + this->Size.x && a.y < this->Upper.y)
			{
				Point2D necorner{Upper.x + Size.x, Upper.y};
				a.DistanceOfTwo(necorner);
				point_distance = a.ptopdistance;
			}
			//6.
			else if(a.x > this->Upper.x + this->Size.x && a.y > this->Upper.y + this->Size.y)
			{
				Point2D secorner{Upper.x + Size.x, Upper.y + Size.y};
				a.DistanceOfTwo(secorner);
				point_distance = a.ptopdistance;
			}
		}
	};

	struct CompareRect
	{
	    bool operator() (Rectangle* r1, Rectangle* r2)
	    {
	        return r1->point_distance > r2->point_distance;
	    }
	};


	unsigned int max_levels{30};
	Rectangle* root{nullptr};
	ostream & _rect(Rectangle* r, ostream& o) const; //rectangle kiíratásához akár konzolra, akár fájlba.
	ostream & _points(Rectangle* r, ostream& o) const; //points kiíratásához akár konzolra, akár fájlba.

public:

    Quadtree (Point2D a, Point2D b);
    ~Quadtree();

    bool isempty() const { return root == nullptr; };
    void split(Rectangle* r);
    void destroy(Rectangle* r);
    bool insert(Point2D a);
    bool _insert(Rectangle* r, Point2D a);
    ostream & rect(ostream & o) const;
    ostream & points(ostream& o) const;
    vector<Point2D> kFind(Point2D a, unsigned int k);

};


Quadtree::Quadtree (Point2D a, Point2D b) { root = new Rectangle(a,b,nullptr); }

//A destroyt a rootra hívom, ezáltal majd rekurzívan az egész fára meghívódik.
Quadtree::~Quadtree() { destroy(root); }

void Quadtree::destroy(Rectangle* r)
{
	//rekurzív destroy r rectangle gyerekeire.
	if (r != nullptr)
	{
		destroy (r->nw);
		destroy (r->ne);
		destroy (r->sw);
		destroy (r->se);
		delete r; //végül szülő törlése.
	}
}

void Quadtree::split(Rectangle* r)
{
	//Segédváltozók a fa négyfelé osztásához.
	float subWidth{r->Size.x/2};
	float subHeight{r->Size.y/2};
	float x{r->Upper.x};
	float y{r->Upper.y};

	Point2D centre{subWidth, subHeight};

	//meghatározzuk r, a szétosztott rectangle gyerekeinek határait.
	r->nw = new Rectangle (Point2D{x, y}, centre, r);
	r->ne = new Rectangle (Point2D{x + subWidth, y}, centre, r);
	r->sw = new Rectangle (Point2D{x, y + subHeight}, centre, r);
	r->se = new Rectangle (Point2D{x + subWidth, y + subHeight}, centre, r);

	//a szülő pontjait szétosztjuk a gyerekei közt, majd hogy ne legyen duplicate-ek töröljük a szülő elemeit.
	for (auto p: r->points)
	{
		if (r->nw->Contains(p)) r->nw->points.insert(p);
		else if (r->ne->Contains(p)) r->ne->points.insert(p);
		else if (r->sw->Contains(p)) r->sw->points.insert(p);
		else if (r->se->Contains(p)) r->se->points.insert(p);
	}
	r->points.clear();
}

bool Quadtree::insert(Point2D a)
{
	return _insert(root, a);
}


bool Quadtree::_insert(Rectangle* r, Point2D a)
{

	// Rekurzív megoldás miatt kell az első sor, lásd (*)
	// (Ha az éppen nézett r rectangle nem is tartalmazhatná az új pontot, akkor a függvény false-al tér vissza)
	if (!r->Contains(a)) return false;

	if (r->points.size() < r->max_objects)
	{
		//Ha a pontoknak a száma nem haladja meg a megengedhetőt, beszúrjuk az új pontot a szülő node-ba.
		r->points.insert(a);
		return true;
	}
	else
	{
		// (*) Rekurzívan vizsgáljuk, hogy az egyes gyerek node-okba sikerült-e beszúrni az elemet.
		if (r->nw == nullptr) split(r);

		if (_insert(r->nw, a) || _insert(r->ne, a) || _insert(r->sw, a) || _insert(r->se, a))
		{
			return true;
		}
		return false;
	}
}

//A rectangle 4 adatpontjának konzolra/fájlba való kiíratása.
ostream & Quadtree::rect(ostream & o) const
{
	//ez a kivétel nem fordulhat elő, hisz ha már létrehoztam a quadtree-t, a root nem lesz nullptr.
	//van destroy függvény (rekurzív törlés), de jelenleg csak a destruktorban használom~
	if (isempty())
	{
		cerr << "Üres fának az elemeit szeretnéd kiíratni!" << endl;
		exit(1);
	}
	return _rect(root, o);
}

//A fa pontjainak konzolra/fájlba való kiíratása.
ostream & Quadtree::points(ostream& o) const
{
	if (isempty())
	{
		cerr << "Üres fának az elemeit szeretnéd kiíratni!" << endl;
		exit(1);
	}

	return _points(root, o);
}

ostream & Quadtree::_rect(Rectangle* r, ostream & o) const
{

	o  << r->Upper.x << " " << r->Upper.y << " " << r->Size.x << " " << r->Size.y << endl;
	if (r->nw != nullptr)
	{
		_rect(r->nw, o);
	}

	if (r->ne != nullptr)
	{
		_rect(r->ne, o);
	}

	if (r->sw != nullptr)
	{
		_rect(r->sw, o);
	}

	if (r->se != nullptr)
	{
		_rect(r->se, o);
	}

	return o;
}

ostream & Quadtree::_points(Rectangle* r, ostream & o) const
{
	//Megvizsgáljuk, hogy levélelemről van-e szó és csak akkor íratunk ki.
	if (r->nw == nullptr)
	{
		for (auto p: r->points)
		{
			o << p.x << " " << p.y << endl;
		}
	}

	if (r->nw != nullptr)
	{
		_points(r->nw, o);
	}

	if (r->ne != nullptr)
	{
		_points(r->ne, o);
	}

	if (r->sw != nullptr)
	{
		_points(r->sw, o);
	}

	if (r->se != nullptr)
	{
		_points(r->se, o);
	}
	return o;
}

///--------------------------------------------- KNN KÉT MEGVALÓSÍTÁSA ---------------------------------------------
/*A fentről lefelé haladó módszert használom mindkét algoritmusban. (a 'knn paper' alapján implementálva)
Ezzel csak az a gond, hogy a benne levő priority queue konténer-ben két fajta elemet is tárol. (pontokat és rectangleket)
a knn-be beadott ponttól való távolság alapján sortolva.
Nyilván ez c++-csak bajosan lehetséges. (Bár ennek is utánanéztem, boost::any ötlet jött fel például)

Ezért én itt két konténert is segítségül veszek:
		1.) megvalósítás: egy priority queue és egy vector
		2.) megvalósítás: két priority queue
Az első priority RectsByPriority-ban rectangle-ket tárolok a vizsgált ponttól való távolságuk alapján
A másodikban pontokat, a vizsgált ponttól való távolság alapján

Mindkét konténer felhasználásának a lényege, hogy írhatunk hozzá komparátort, lásd feljebb. (lambdával persze lehetett volna okosabban is)

~ Megejtettem azt a hibát, hogy a while ciklust addig futtattam, míg a pont pr. queue-ban megvolt a keresett szomszéd-szám
ez azért bajos, mert így további közelebbi rectangle-ket nem fog "szétbontani elemeire", így nem a megfelelő közeli elemeket találja meg.
Ekkor felmerül az a gond, hogy akkor mégis meddig fusson a ciklus?

if (PointsByPriority.size() >= k && (RectsByPriority.top()->point_distance > PointsByPriority[k].ptopdistance))

Ennek a megoldására ez a sor szolgál.
Viszont, a pr. queue-ben nem lehetséges lekérdezni az utolsó elemet (ill. iterátor sincs rajta definiálva)
Ezért jutottam el végül a vector-priority queue párosításhoz.*/

vector<Point2D> Quadtree::kFind(Point2D a, unsigned int k)
{

	//Az első priority RectsByPriority-ban rectangle-ket tárolok a vizsgált ponttól való távolságuk alapján
	//A másodikban pontokat, a vizsgált ponttól való távolság alapján

	priority_queue <Rectangle*, vector<Rectangle*>, CompareRect> RectsByPriority;
	vector <Point2D> PointsByPriority;

	vector <Point2D> nearest_elements;
	Rectangle* r = root;

	//Kezdetben a RectsByPriority-be bekerül a root.
	RectsByPriority.push(r);

	while(!RectsByPriority.empty())
	{
		Rectangle* e = RectsByPriority.top();

		//Ha a Rectangle Priority Queue elején üres levélelemek leledzenek, azokat kitörli.
		while (e->points.empty())
		{
			RectsByPriority.pop();
			e = RectsByPriority.top();
		}

		//Ha már találtunk k elemet, akkor:
		//Ellenőrizzük, hogy a Rectangle Pr. Queue legfelső elemének távolásága nagyobb-e a k-adik megtalált pont távolságánál
		//Ha igen, akkor értelemszerűen abort.
		if (PointsByPriority.size() >= k && (RectsByPriority.top()->point_distance > PointsByPriority[k].ptopdistance))
		{
			break;
		}

		//Ha levélelem kerül a Rectangle Priority Queue elejére, akkor beveszi a Point Pr. Q.-be az elemeit. (prioritás alapján)
		if (e->IsLeaf())
		{
			for (auto p: e->points)
				{
					p.DistanceOfTwo(a);
					PointsByPriority.push_back(p);
					sort(PointsByPriority.begin(), PointsByPriority.end(), ComparePointsD());
				}
			RectsByPriority.pop();
		}
		//Ha nem levélelemet találtunk meg, akkor gyerekeire bontjuk szét, majd önmagát töröljük a Rectangle Priority Queue-ból.
		else if (!e->IsLeaf())
		{
			Rectangle* h = e;
			RectsByPriority.pop();
			h->nw->DistToPoint(a);
			RectsByPriority.push(h->nw);
			h->ne->DistToPoint(a);
			RectsByPriority.push(h->ne);
			h->sw->DistToPoint(a);
			RectsByPriority.push(h->sw);
			h->se->DistToPoint(a);
			RectsByPriority.push(h->se);
		}

	}
	//Végül a visszatérési vektorba a Point Pr. Q. megfelelő elemeit "áttöltjük".
	for (unsigned int i = 0; i < k; i++)
	{
		nearest_elements.push_back(PointsByPriority[i]);
	}

	return nearest_elements;
}


/*vector<Point2D> Quadtree::kFind(Point2D a, unsigned int k)
{
	//A fentről lefelé haladó módszert használom. (a 'knn paper' alapján implementálva)

	//Az első priority RectsByPriority-ban rectangle-ket tárolok a vizsgált ponttól való távolságuk alapján
	//A másodikban pontokat, a vizsgált ponttól való távolság alapján

	priority_queue <Rectangle*, vector<Rectangle*>, CompareRect> RectsByPriority;
	priority_queue <Point2D, vector<Point2D>, ComparePointsD> PointsByPriority;

	vector <Point2D> nearest_elements;
	Rectangle* r = root;

	//Kezdetben a RectsByPriority-be bekerül a root.
	RectsByPriority.push(r);

	while(!RectsByPriority.empty())
	{
		Rectangle* e = RectsByPriority.top();

		//Ha a Rectangle Priority Queue elején üres levélelem leledzenek, azokat kitörli.
		while (e->points.empty())
		{
			RectsByPriority.pop();
			e = RectsByPriority.top();
		}

		//Ha levélelem kerül a Rectangle Priority Queue elejére, akkor beveszi a Point Pr. Q.-be az elemeit.
		if (e->IsLeaf())
		{
			for (auto p: e->points)
				{
					p.DistanceOfTwo(a);
					PointsByPriority.push(p);
				}
			RectsByPriority.pop();
		}
		//Ha nem levélelemet találtunk meg, akkor gyerekeire bontjuk szét, majd önmagát töröljük a Rectangle Priority Queue-ból.
		else if (!e->IsLeaf())
		{
			Rectangle* h = e;
			RectsByPriority.pop();
			h->nw->DistToPoint(a);
			RectsByPriority.push(h->nw);
			h->ne->DistToPoint(a);
			RectsByPriority.push(h->ne);
			h->sw->DistToPoint(a);
			RectsByPriority.push(h->sw);
			h->se->DistToPoint(a);
			RectsByPriority.push(h->se);
		}

	}
	//Végül a visszatérési vektorba a Point Pr. Q. megfelelő elemeit áttöltjük.
	for (unsigned int i = 0; i < k; i++)
	{
		nearest_elements.push_back(PointsByPriority.top());
		PointsByPriority.pop();
	}

	return nearest_elements;
}*/


#endif // BIN_TREE_HPP_INCLUDED
