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
	
	#ifdef DEBUG
	move(this->pos.y, this->pos.x*2 + 50);
	printw("%d",aliveNeighbors);
	#endif

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
	printw((this->alive ? "█": "░"));
#ifdef DEBUG
	move(this->pos.y + offY, this->pos.x + offX + 50);
	printw((this->wasAlive ? "█": "░"));
#endif
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
					int nJ = j + subY; 
					int nI = i + subX;
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

void A(Cell *cell){
	cell->alive = ( (cell->pos.x%10) > 8) && (bool)(rand()%4);
}
void B(Cell *cell){
	cell->alive = !(rand() % 4);
}

bool isPerfectSquare(int x) 
{ 
    int s = sqrt(x); 
    return (s*s == x); 
} 
  
// Returns true if n is a Fibinacci Number, else false 
bool isFib(int n) 
{ 
    // n is Fibinacci if one of 5*n*n + 4 or 5*n*n - 4 or both 
    // is a perferct square 
    return isPerfectSquare(5*n*n + 4) || 
           isPerfectSquare(5*n*n - 4); 
} 

void C(Cell *cell){
	cell->alive = isFib(cell->pos.x * cell->pos.y);
}

int main (void)
{
	setlocale(LC_ALL,"");
	initscr();
	noecho();

	getmaxyx(stdscr, termCols, termRows);
	srand(time(0));
	
	World myA = World(2,2,90,15,A); myA.draw();
	World myB = World(2,22,90,15,B); myB.draw();
	World myC = World(2,42,90,15,C); myC.draw();

	move(0,0);
	printw("Press Enter to begin!");

	refresh();
	
	char input;
	while((input=getch())!='\n');
	
	while(1){
		getmaxyx(stdscr, termCols, termRows);
		
		myA.update(); myA.draw();
		myB.update(); myB.draw();
		myC.update(); myC.draw();

		refresh();
		wait(0.1);
		
	}
	
	endwin();
	return 0;
}
