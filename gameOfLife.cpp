#include <ncursesw/ncurses.h>
#include <iostream>
#include <unistd.h>
#include <locale.h>
#include <ctime>
#include <cstdlib>
#include <math.h> 

using namespace std;

int termCols;
int termRows;

void wait( float seconds){
	usleep ( seconds * 1e6 );
}

struct Vect {
	int x;
	int y;
	Vect(int x, int y) : x(x), y(y) {} ;
	Vect(){};
};

struct Cell {
	Cell(int,int,bool);
	bool alive;
	bool wasAlive;
	int lifeSpan = 0;
	Vect pos;
	Cell* neighbors[8] = {NULL};
	void process();
	void draw(int,int);
	void relocate(int x, int y);
	void saveState();
};

Cell::Cell(int x=0, int y=0, bool alive=true){
	this->pos.x = x;
	this->pos.y = y;
	this->alive = alive;
}

void Cell::process(){
	int aliveNeighbors = 0;

	for(int i=0; i < 8; i++){
		Cell* neighbor = this->neighbors[i];
		if(neighbor != NULL){
			if(neighbor->wasAlive){
				aliveNeighbors += 1;
			}
		}
	}
	
	if (this->alive and (aliveNeighbors < 2 or aliveNeighbors > 3)){
		this->alive = false;
	}
	if (!this->alive and aliveNeighbors == 3){
		this->alive = true;
	}
	if(this->alive){
		this->lifeSpan += 1;
	} else {
		this->lifeSpan = 0;
	}
}

void Cell::saveState(){
	this->wasAlive = this->alive;
}

void Cell::draw(int offX, int offY){
	move(this->pos.y + offY, this->pos.x + offX);
	bool state [2] = {this->alive, this->wasAlive};
	//printw(( this->alive ? "█": (this->wasAlive ? "▓" : "░" )));
	printw(( this->alive ? "█": "░" ));
}

struct World {
	Vect size;
	Vect pos;
	int cellCount;
	Cell * cells;
	World(int, int, int, int, void(&)(Cell*));
	~World();
	void analyze();
	void update();
	void draw();
	void (*excite)(Cell*);
	int gen;
	int aliveCount;
	int eqGen;
};
int modulo(int x,int N){
    return (x % N + N) %N;
}

World::World(int x, int y, int w, int h, void (&excitor)(Cell*)){
	this->pos.x = x;
	this->pos.y = y;
	this->size.x = w;
	this->size.y = h;
	this->cellCount = w*h;
	this->cells = new Cell[this->cellCount];
	this->excite = excitor;

	// for all cells
	for(int j=0; j<h; j++){
		for(int i=0; i<w; i++){
	
			// focus on a cell
			Cell *focusC = &this->cells[j*w+i];
			// Tell this cell its position
			focusC->pos.x = i;
			focusC->pos.y = j;
			
			// Find all its neighbors
			int k=0;
			for(int subY = -1; subY <= 1; subY++){
				for(int subX = -1; subX <= 1; subX++){
					int nJ = modulo((j + subY), this->size.y); 
					int nI = modulo((i + subX), this->size.x);
					//int nJ = (j + subY); 
					//int nI = (i + subX);
					// exclude itself from the list of neighbors
					if (!(nJ == j and nI==i) and nJ > -1 and nJ < h and nI > -1 and nI < w){
						focusC->neighbors[k] = &this->cells[nJ*w+nI];
						k++;
					}
				}
			}
			
			// Give it a random state
			this->excite(focusC);
		}
	}
	this->gen = 1;
	this->eqGen = -1;
}
World::~World(){
	delete[] this->cells;
}

void World::analyze(){
	// get the population
	int prevAlive = this->aliveCount;
	this->aliveCount = 0;
	for(int i=0; i< (this->cellCount); i++){
		if(this->cells[i].alive){
			this->aliveCount += 1;
		}
	}
	// check if system has reached equilibrium
	if(prevAlive != this->aliveCount){
		this->eqGen = this->gen;
	}
}

void World::update(){
	// Save the current Gen as the previous gen
	for(int i=0; i< (this->cellCount); i++){
		this->cells[i].saveState();
	}
	//process the previous Gen to make the next Gen
	for(int i=0; i< (this->cellCount); i++){
		this->cells[i].process();
	}
	this->gen += 1;
	this->analyze();
}
void World::draw(){
	for(int i=0; i< (this->cellCount); i++){
		this->cells[i].draw(this->pos.x, this->pos.y);
	}
	this->analyze();
	move(this->pos.y+this->size.y, this->pos.x);
	printw("Generation: %d \t", this->gen);
	printw("Cells: %d \t", this->cellCount);
	printw("Population: %d  \t", this->aliveCount);
	printw("Equilibrium: %d\n", this->eqGen);
}

bool match(Vect input, int n, Vect *pointSet){
	for(int i = 0; i < n; i++){
		if(input.x == pointSet[i].x and input.y == pointSet[i].y){
			return true;
		}
	}
	return false;
}
void B(Cell *cell){
	int n = 7;
	Vect points[n] = {Vect(11,11), Vect(12,11), Vect(12,13), Vect(14,12), Vect(15,11), Vect(16,11), Vect(17,11)}; 
	
	cell->alive = rand()%2;

}

void A(Cell *cell){
	int x = cell->pos.x;
	int y = cell->pos.y;
	
	srand(x*y%50);
	cell->alive = rand()%2;
}

int main (void)
{
	setlocale(LC_ALL,"");
	initscr();
	noecho();

	getmaxyx(stdscr, termCols, termRows);
	srand(time(0));
	
	World myA = World(2,2,100,100,B); myA.draw();
	//World myB = World(2,22,90,15,B); myB.draw();

	move(0,0);
	printw("Press Enter to begin!");

	refresh();
	
	char input;
	while((input=getch())!='\n');
	
	while(1){
		getmaxyx(stdscr, termCols, termRows);
		
		myA.update(); myA.draw();
		//myB.update(); myB.draw();

		refresh();
		wait(0.05);
		
	}
	
	endwin();
	return 0;
}
