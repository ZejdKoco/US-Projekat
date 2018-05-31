#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <queue>
#include <cctype>
#include <cstdlib>
#include <ctime>
#include <list>
#include <stdexcept>

using namespace std;

static const char screenPanels[] = {30, 30, 60};
static const char screenHeight = 40;

typedef struct coord_ {
	coord_() : col(0), row(0) { }
	coord_(char c, char r) : col(c), row(r) { }

	char col;
	char row;
} coord_t;


class Game {
public:
	Game() : finished(false) { }

	bool gameOver() {
		return finished;
	}

	virtual void gameUpdate() = 0;
	virtual ~Game() { }

protected:
	void gameEnd() {
		finished = true;
	}

private:
	bool finished;
};


class BS_Tile {
public:
	// Constructor
	BS_Tile() : col(0), row(0) { }
	BS_Tile(char c, char r) : col(c), row(r) { }

	char getCol() {
		return col;
	}

	char getRow() {
		return row;
	}

	bool valid() {
		return col;
	}

private:
	char col, row;
};


class BS_BoardTile : public BS_Tile {
public:
	BS_BoardTile() : hit(false), ship(false) { }
	BS_BoardTile(char c, char r) : BS_Tile(c, r), hit(false), ship(false) { }

	void setHit() {
		hit = true;
	}

	bool getHit() {
		return hit;
	}

	bool getShip() {
		return ship;
	}

	void setShip() {
		ship = true;
	}
private:
	bool hit;
	bool ship;
};

class BS_ShipTile : public BS_Tile {
public:
	BS_ShipTile() : h(false) { }
	BS_ShipTile(char c, char r) : BS_Tile(c, r), h(false) { }

	void setHit() {
		h = true;
	}

	bool getHit() {
		return h;
	}
private:
	bool h;
};

class Ship {
public:
	typedef struct {
		std::string type;
		char size;
	} shipInfo;

	typedef std::vector<BS_ShipTile>::iterator ship_it;
	Ship(const shipInfo&, char c1, char r1, char c2, char r2);

	std::string type() {
		return info.type;
	}

	char size() {
		return info.size;
	}

	std::vector<BS_ShipTile>::iterator shipBegin() {
		return sections.begin();
	}

	std::vector<BS_ShipTile>::iterator shipEnd() {
		return sections.end();
	}

	bool collision(Ship&);
	bool sunk();
	bool checkHit(char, char);
	bool checkLocation(char, char);
	bool static checkSize(char, char, char, char, char);

private:
	shipInfo info;
	std::vector<BS_ShipTile> sections;
};

class BS_Strategy {
public:
typedef enum {NOCHANCE, NOTLIKELY, MAYBE, LIKELY} chance_t;

typedef enum {NONE, UP, DOWN, LEFT, RIGHT} dir_t;
	BS_Strategy() : up(MAYBE), down(MAYBE), left(MAYBE), right(MAYBE),successful(true), last_dir(NONE) { }
	bool valid();
	void update(char c, char r);
	bool getMove(char *c, char *r);
private:
	void reset();
	void set(char c, char r);
	chance_t up, down, left, right;
	dir_t last_dir;
	bool successful;
	coord_t firstTry;
	coord_t lastTry;
};

void BS_Strategy::update(char c, char r) {

	if (!valid()) {
		set(c, r);
	} else {
		successful = true;
		if (UP == last_dir || DOWN == last_dir) {
			left = NOTLIKELY;
			right = NOTLIKELY;
		} else {
			up = NOTLIKELY;
			down = NOTLIKELY;
		}
	}
}

bool BS_Strategy::valid()
{
	if (firstTry.col) {
		return true;
	} else {
		return false;
	}
}

void BS_Strategy::reset() {
	firstTry.col = 0;
	firstTry.row = 0;
	lastTry.col = 0;
	lastTry.row = 0;
	up = MAYBE;
	down = MAYBE;
	left = MAYBE;
	right = MAYBE;
	successful = true;
	last_dir = NONE;
}

void BS_Strategy::set(char c, char r) {
	firstTry.col = c;
	firstTry.row = r;
	lastTry.col = c;
	lastTry.row = r;
}

bool BS_Strategy::getMove(char *c, char *r)
{
	if (!successful) {
		if (UP == last_dir) {
			up = NOTLIKELY;
		} else if (DOWN == last_dir) {
			down = NOTLIKELY;
		} else if (LEFT == last_dir) {
			left = NOTLIKELY;
		} else if (RIGHT == last_dir) {
			right = NOTLIKELY;
		}
	}

	bool success = false;
	while(!success) {
		if (MAYBE == up) {

			if (UP != last_dir) {
				last_dir = UP;
				lastTry = firstTry;
			}

			if (1 == lastTry.row) {
				up = NOCHANCE;
			} else {
				--lastTry.row;
				success = true;
			}
		} else if (MAYBE == down) {

			if (DOWN != last_dir) {
				last_dir = DOWN;
				lastTry = firstTry;
			}

			if (10 == lastTry.row) {
				down = NOCHANCE;
			} else {
				++lastTry.row;
				success = true;
			}
		} else if (MAYBE == left) {

			if (LEFT != last_dir) {
				last_dir = LEFT;
				lastTry = firstTry;
			}

			if ('A' == lastTry.col) {
				left = NOCHANCE;
			} else {
				--lastTry.col;
				success = true;
			}
		} else if (MAYBE == right) {

			if (RIGHT != last_dir) {
				last_dir = RIGHT;
				lastTry = firstTry;
			}

			if ('J' == lastTry.col) {
				right = NOCHANCE;
			} else {
				++lastTry.col;
				success = true;
			}
		} else {
			reset();
			return false;
		}
	}
	*c = lastTry.col;
	*r = lastTry.row;
	successful = false;

	return true;
}

bool Ship::checkSize(char size, char col1, char row1, char col2, char row2)
{
	if (row1 == row2) {
		if (col1 > col2) {
			if ((col1 - col2) + 1 == size) {
				return true;
			}
		} else {
			if ((col2 - col1) + 1 == size) {
				return true;
			}
		}
	}

	if (col1 == col2) {
		if (row1 > row2) {
			if ((row1 - row2) + 1 == size) {
				return true;
			}
		} else {
			if ((row2 - row1) + 1 == size) {
				return true;
			}
		}
	}

	return false;
}

void buildShipSections(vector<BS_ShipTile>& tiles, char col1, char row1, char col2, char row2)
{
	int i;

	if (row1 == row2) {
		if (col1 < col2) {
			for (i = col1; i <= col2; ++i) {
				tiles.push_back(BS_ShipTile(i,row1));
			}
		} else {
			for (i = col2; i <= col1; ++i) {
				tiles.push_back(BS_ShipTile(i, row1));
			}
		}
	} else {
		if (row1 < row2) {
			for (i = row1; i <= row2; ++i) {
				tiles.push_back(BS_ShipTile(col1, i));
			}
		} else {
			for (i = row2; i <= row1; ++i) {
				tiles.push_back(BS_ShipTile(col1, i));
			}
		}
	}
}

Ship::Ship(const shipInfo& i, char c1, char r1, char c2, char r2)
{
	info = i;
	buildShipSections(sections, c1, r1, c2, r2);
}

bool Ship::checkHit(char col, char row)
{
	for (ship_it it = sections.begin(); it != sections.end(); ++it) {
		if ((*it).getCol() == col && (*it).getRow() == row) {
			(*it).setHit();
			return true;
		}
	}
	return false;
}

bool Ship::sunk()
{
	for(ship_it it = sections.begin(); it != sections.end(); ++it) {
		if ((*it).getHit() == false)
			return false;
	}
	return true;
}

bool Ship::checkLocation(char col, char row)
{
	for (ship_it it = sections.begin(); it != sections.end(); ++it) {
		if ((*it).getCol() == col && (*it).getRow() == row)
			return true;
	}
	return false;
}

bool Ship::collision(Ship& s)
{
	bool ret = false;
	for (ship_it it1 = s.sections.begin(); it1 != s.sections.end(); ++it1) {
		for (ship_it it2 = sections.begin(); it2 != sections.end(); ++it2) {
			if(	((*it1).getCol() == (*it2).getCol()) &&
				((*it1).getRow() == (*it2).getRow())	) {
				ret = true;
			}
		}
	}
	return ret;
}

class ConsoleDisplay {
public:
	typedef std::vector<std::vector<std::string> >::iterator screenIt_t;
	typedef std::vector<std::string>::iterator panelIt_t;
	ConsoleDisplay(const char w[], const char panels, const char height);
	void clear();
	void refresh();
	void write(const std::string& str);
	void write(const unsigned char panel, const std::string& str);
	void write(const unsigned char panel, const unsigned char row, const std::string& str);
private:
	const std::vector<char> w;
	const char h, p;
	char lastRow;
	std::vector< std::vector<std::string> > screen;
	std::vector<std::string> stream;
};

ConsoleDisplay::ConsoleDisplay(const char widths[], const char panels, const char height) :
				w(widths, widths + panels), h(height), p(panels), lastRow(0)
{
	for (int i = 0; i < p; ++i)
		screen.push_back(vector<string>());

	for (screenIt_t s_it = screen.begin(); s_it != screen.end(); ++s_it)
		for (int i = 0; i < h; ++i)
			(*s_it).push_back(string());
}

void ConsoleDisplay::clear()
{
	for (screenIt_t s_it = screen.begin(); s_it != screen.end(); ++s_it)
		for (panelIt_t p_it = (*s_it).begin(); p_it != (*s_it).end(); ++p_it)
			(*p_it).clear(); // Clear string

	lastRow = 0;

	stream.clear();
}

void ConsoleDisplay::refresh()
{
	system("clear");

	vector<string> screenOut(lastRow + 1);

	char panel = 0;
	for (screenIt_t s_it = screen.begin(); s_it != screen.end(); ++s_it) {
		panelIt_t p_it = (*s_it).begin();
		for (panelIt_t scrOut_it = screenOut.begin(); scrOut_it != screenOut.end(); ++scrOut_it) {
			try {
				(*scrOut_it) += ((*p_it) + string(w[panel] - (*p_it).length(), ' ') + "  ");
			} catch (const length_error& le) {
				(*scrOut_it) += ((*p_it) + "  ");
			}
			++p_it;
		}
		++panel;
	}
	for (panelIt_t p_it = screenOut.begin(); p_it != screenOut.end(); ++p_it)
		cout << *p_it << endl;
	for (panelIt_t st_it = stream.begin(); st_it != stream.end(); ++st_it)
		cout << *st_it << endl;
}

void ConsoleDisplay::write(const unsigned char panel, const unsigned char row, const string& str)
{
	if (panel >= p)
		return;
	else if (row >= h)
		return;
	else if (w[panel] < (screen[panel][row].length() + str.length()))
		return;
	else
		screen[panel][row] += str;

	if (lastRow < row)
		lastRow = row;
}

void ConsoleDisplay::write(const unsigned char panel, const string& str)
{
	if (p <= panel)
		return;
	else if (str.length() > w[panel])
		return;

	char row = 0;
	panelIt_t p_it = screen[panel].begin();
	while ((p_it != screen[panel].end()) && (!(*p_it).empty())) {
		++p_it;
		++row;
	}

	if (h <= row)
		return;

	(*p_it) += str;

	if (lastRow < row)
		lastRow = row;
}

void ConsoleDisplay::write(const string& str)
{
	stream.push_back(str);
}

class BS_Board {
public:
	static int const boardSize = 10;

	const std::vector<Ship::shipInfo> shipTypes = {{ "Aircraft Carrier", 5},{ "Battle Ship", 4},{ "Destroyer", 3},{ "Submarine", 3},{ "Patrol Boat", 2}};

	typedef enum {AIRCRAFT, BATTLE, DESTROYER, SUB, PATROL} ship_t;
	BS_Board(std::istream& i, std::ostream& o);

	void placeShips(ConsoleDisplay& display);
	void placeShips();
	bool makeAttempt(char c, char r);
	bool checkShot(char c, char r, std::string& str);
	void markHit(char c, char r);
	bool fleetSunk();
	void writeShipGrid(ConsoleDisplay& display);
	void writeAttackGrid(ConsoleDisplay& display);
private:
	bool shipCollision(Ship&);
	void shipPlacementPrompt(const Ship::shipInfo&, ConsoleDisplay& display);
	void shipPlacementRandom(const Ship::shipInfo&);
	void markBoard(Ship&);
	std::istream& in;
	std::ostream& out;
	std::vector<Ship> ships;
	std::vector<BS_BoardTile> shipGrid;
	std::vector<BS_BoardTile> attackGrid;
};

string const shipGridTop =     "        Ship Grid      ";
string const attackGridTop = "       Attack Grid     ";
string const boardTop =     "  |A|B|C|D|E|F|G|H|I|J|";
string const rowStart[BS_Board::boardSize] = {
				" 1|",
				" 2|",
				" 3|",
				" 4|",
				" 5|",
				" 6|",
				" 7|",
				" 8|",
				" 9|",
				"10|" };

string const shipNoHit = "O";
string const shipHit = "@";
string const boardNoHit = " ";
string const boardHit = "X";

string const legend =	"---Legenda---   \nO Brod         \n@ Pogodjen brod\nX Promasaj\n";

BS_Board::BS_Board(istream& i, ostream& o) :
			in(i), out(o)
{
	for (int row = 1; row < 1 + boardSize; ++row) {
		for(int col = 'A'; col < 'A' + boardSize; ++col) {
			shipGrid.push_back(BS_BoardTile(col, row));
		}
	}
	for (int row = 1; row < 1 + boardSize; ++row) {
		for(int col = 'A'; col < 'A' + boardSize; ++col) {
			attackGrid.push_back(BS_BoardTile(col, row));
		}
	}
}

string displayTile(BS_BoardTile& tile)
{
	int sel = ((int)tile.getHit() << 1) | (int)tile.getShip(); // 0 - 3
	switch(sel) {
	case 0:
		return boardNoHit + "|";
		break;
	case 1:
		return shipNoHit + "|";
		break;
	case 2:
		return boardHit + "|";
		break;
	case 3:
		return shipHit + "|";
		break;
	}
}

void BS_Board::writeShipGrid(ConsoleDisplay& display)
{
	vector<BS_BoardTile>::iterator b_it;
	int row = 0;
	display.write(0, row++, shipGridTop);
	display.write(0, row++, boardTop);

	int i = 0;
	for (b_it = shipGrid.begin(); b_it != shipGrid.end(); ++b_it) {
		if ((*b_it).getCol() == 'A') {
			display.write(0, row, rowStart[i++]);
			display.write(0, row, displayTile(*b_it));
		} else if ((*b_it).getCol() == 'J') {
			display.write(0, row, displayTile(*b_it));
			++row;
		} else {
			display.write(0, row, displayTile(*b_it));
		}
	}
	display.write(legend);
}

void BS_Board::writeAttackGrid(ConsoleDisplay& display)
{
	vector<BS_BoardTile>::iterator a_it;
	int row = 0;
	display.write(1, row++, attackGridTop);
	display.write(1, row++, boardTop);

	int i = 0;
	for (a_it = attackGrid.begin(); a_it != attackGrid.end(); ++a_it) {
		if ((*a_it).getCol() == 'A') {
			display.write(1, row, rowStart[i++]);
			display.write(1, row, displayTile(*a_it));
		} else if ((*a_it).getCol() == 'J') {
			display.write(1, row, displayTile(*a_it));
			++row;
		} else {
			display.write(1, row, displayTile(*a_it));
		}
	}
}

class BS_Player {
public:
	BS_Player() { }
	BS_Player(std::string name) : n(name) { }

	std::string name() {
		return n;
	}

	virtual void move(char* c, char* r, BS_Board& board) = 0;

	virtual ~BS_Player() { }

protected:
	std::string n;
};


class BS_ComputerPlayer : public BS_Player {

public:
	BS_ComputerPlayer(std::string name = "AI");

	void move(char *c, char *r, BS_Board& board);

	void update(char c, char r) {
		strategy.update(c, r);
	}
private:
	std::list<coord_t> coords;
	BS_Strategy strategy;
};

#define GENIUS_COMPUTER 1

BS_ComputerPlayer::BS_ComputerPlayer(std::string name) : BS_Player(name)
{
	for (int r = 1; r < 1 + BS_Board::boardSize; ++r) {
		for (int c = 'A'; c < 'A' + BS_Board::boardSize; ++c) {
			coords.push_back(coord_t(c, r));
		}
	}
	srand(time(0));
}

void BS_ComputerPlayer::move(char *c, char *r, BS_Board& board)
{
#if DUMB_COMPUTER
	static int i = 1;
	static int j = 'A';

	if ('A' + BS_Board::boardSize == j) {
		j = 'A';
		*c = j;
		*r = ++i;
	} else {
		*c = j++;
		*r = i;
	}
#elif SMART_COMPUTER
	list<coord_t>::iterator c_it = coords.begin();
	int count = (rand() % coords.size());
	while (count--) {
		c_it++;
	}
	*c = (*c_it).col;
	*r = (*c_it).row;
	coords.erase(c_it);
#elif GENIUS_COMPUTER
	list<coord_t>::iterator c_it;

	bool success = false;

	while (!success && strategy.valid()) {
		strategy.getMove(c, r);
		for (c_it = coords.begin(); c_it != coords.end(); ++c_it) {
			if (*c == (*c_it).col && *r == (*c_it).row) {
				success = true;
				coords.erase(c_it);
				break;
			}
		}
	}

	if (!success) {

		int count = (rand() % coords.size());
		c_it = coords.begin();
		while (count--) {
			c_it++;
		}
		*c = (*c_it).col;
		*r = (*c_it).row;
		coords.erase(c_it);
	}
#endif
}

class BS_HumanPlayer : public BS_Player {

public:
	BS_HumanPlayer(std::istream& i = std::cin, std::ostream& o = std::cout);

	void move(char*, char*, BS_Board& board);
private:
	bool readUserShotInput(char*, char*);
	std::istream& in;
	std::ostream& out;
};

BS_HumanPlayer::BS_HumanPlayer(istream& i, ostream& o) : in(i), out(o)
{
	string str;
	do {
		out << "Dobrodosli u igru Potapanje brodova!!! Unesite ime (max 20 znakova)?: ";
		in >> str;
	} while (str.length() > 20);

	n = str;
}

bool BS_HumanPlayer::readUserShotInput(char *col, char *row)
{
	char c;
    	string str;

    	while(in.get(c) && isdigit(c)) {
        	str.push_back(c);
	}

    	*row = atoi(str.c_str());

	if (*row > 10 || *row < 1) {
		return false;
	}

	if (isalpha(c)) {
		*col = toupper(c);
	} else {
		return false;
	}

	if (*col < 'A' || *col > 'J') {
		return false;
	}

	return true;
}

void BS_HumanPlayer::move(char *c, char *r, BS_Board& board)
{
	bool success = false;
	do {
		out << "Enter your shot (ex. 1A):";

		while(in.get() != '\n');

		if (!readUserShotInput(c, r)) {
			continue;
		}

		success = board.makeAttempt(*c, *r);

	} while (!success);
}


class BattleShip : public Game {
	friend class BS_HumanPlayer;
	friend class BS_ComputerPlayer;
public:
	BattleShip(std::istream&, std::ostream&);
	void gameUpdate();
private:
	std::istream& in;
	std::ostream& out;
	ConsoleDisplay display;
	BS_Board p1Board, p2Board;
	BS_HumanPlayer p1;
	BS_ComputerPlayer p2;
};

BattleShip::BattleShip(istream& input, ostream& output) : 	in(input), out(output), display(screenPanels, sizeof(screenPanels), screenHeight),p1Board(in, out),p2Board(in, out),p1(in, out),p2()
{
	p1Board.placeShips(display);
	p2Board.placeShips();
	display.clear();
}

void BattleShip::gameUpdate()
{
	char c, r;
	string str;
	p1Board.writeShipGrid(display);
	p1Board.writeAttackGrid(display);
	display.refresh();

	p1.move(&c, &r, p1Board);

	display.clear();

	if (p2Board.checkShot(c, r, str)) {
		display.write(2, p1.name() + " pogodio " + p2.name()+ " " + str);
		p1Board.markHit(c, r);
	}

	if (p2Board.fleetSunk()) {
		p1Board.writeShipGrid(display);
		p1Board.writeAttackGrid(display);
		display.write(2, p1.name() + " pobijedio!!!");
		display.refresh();
		gameEnd();
		return;
	}

	p2.move(&c, &r, p2Board);
	if (p1Board.checkShot(c, r, str)) {
		display.write(2, p2.name() + " pogodio " + p1.name() + " " + str);
		p2Board.markHit(c, r);
		p2.update(c, r);
	}

	if (p1Board.fleetSunk()) {
		p1Board.writeShipGrid(display);
		p1Board.writeAttackGrid(display);
		display.write(2, p2.name() + " pobijedio!!!");
		display.refresh();
		gameEnd();
		return;
	}
}

bool readUserShipInput(istream& in, char *col1, char *row1, char *col2, char *row2)
{
	char c;
    string str;

    while(in.get(c) && isdigit(c))
        str.push_back(c);

    *row1 = atoi(str.c_str());

    str.clear();

    if (isalpha(c)) {
        *col1 = toupper(c);
    } else {
        return false;
	}

    in.get(c);
    if (c != ' ')
        return false;

    while(in.get(c) && isdigit(c))
        str.push_back(c);

    *row2 = atoi(str.c_str());

    str.clear();

    if (isalpha(c)) {
        *col2 = toupper(c);
    } else {
        return false;
	}

	return true;
}

bool offBoard(char c1, char r1, char c2, char r2)
{
	if(	r1 < 1 || r1 > 10 || r2 < 1 || r2 > 10 ||
		c1 < 'A' || c1 > 'J' || c2 < 'A' || c2 > 'J')
		return true;
	else
		return false;
}

bool BS_Board::shipCollision(Ship& ship)
{
	vector<Ship>::iterator vs_it;

	for (vs_it = ships.begin(); vs_it != ships.end(); ++vs_it) {
		if ((*vs_it).collision(ship)) {
			return true;
		}
	}

	return false;
}

void BS_Board::markBoard(Ship& ship)
{
	vector<BS_ShipTile>::iterator s_it;

	for (s_it = ship.shipBegin(); s_it != ship.shipEnd(); ++s_it) {

		int i = ((*s_it).getRow() - 1) * boardSize;
		i = i + (*s_it).getCol() - 'A';

		shipGrid[i].setShip();
	}
}

void BS_Board::shipPlacementPrompt(const Ship::shipInfo& info, ConsoleDisplay& display)
{
	char c1, r1, c2, r2;

	bool success = false;

	do {
		display.clear();
		writeShipGrid(display);
		display.refresh();
		out << "Unesite poziciju broda oznacavajuci pocetnu " <<
			"i krajnju tacku broda. Npr. Patrol Boat (velicina 2): 1A 1B\n" << endl;

		out << info.type << "(velicina " << (int)info.size << "):";

		while(in.get() != '\n');

		if (!readUserShipInput(in, &c1, &r1, &c2, &r2)) {
			continue;
		}

		if (offBoard(c1, r1, c2, r2)) {
			continue;
		}

		if (!Ship::checkSize(info.size, c1, r1, c2, r2)) {
			continue;
		}

		Ship s(info, c1, r1, c2, r2);
        	if (shipCollision(s)) {
                	continue;
        	} else {
			ships.push_back(s);
			markBoard(s);
			success = true;
		}

	} while (!success);
}

void BS_Board::shipPlacementRandom(const Ship::shipInfo& info)
{
	static std::list<coord_t> coords;
	static bool initialized = false;
	std::list<coord_t>::iterator c_it;

	if (!initialized) {
        	for (int r = 1; r < 1 + BS_Board::boardSize; ++r) {
                for (int c = 'A'; c < 'A' + BS_Board::boardSize; ++c) {
                    coords.push_back(coord_t(c, r));
                }
        	}
		initialized = true;
        srand(time(0));
	}

	bool success = false;
	while (!success) {
		int count = (rand() % coords.size());
		c_it = coords.begin();
		while (count--) {
			c_it++;
		}
		char c1 = (*c_it).col;
		char r1 = (*c_it).row;

		char dirSeed = rand() % 4;
		int i = 0;
		do {
			char c2, r2;
			char direction = (dirSeed + i) % 4;
			switch (direction) {
			case 0:
				c2 = c1;
				r2 = (r1 - info.size) + 1;
				break;
			case 1:
				c2 = c1;
				r2 = (r1 + info.size) - 1;
				break;
			case 2:
				c2 = (c1 - info.size) + 1;
				r2 = r1;
				break;
			case 3:
				c2 = (c1 + info.size) - 1;
				r2 = r1;
				break;
			default:
				while(1);
			}

			if (offBoard(c1, r1, c2, r2)) {
				continue;
			}

			if (!Ship::checkSize(info.size, c1, r1, c2, r2)) {
				continue;
			}

			Ship s(info, c1, r1, c2, r2);
			if (shipCollision(s)) {
				continue;
			} else {
				ships.push_back(s);
				markBoard(s);
				coords.erase(c_it);
				success = true;
			}
		} while (!success && ++i < 4);
	}
}

void BS_Board::placeShips(ConsoleDisplay& display)
{
	vector<Ship::shipInfo>::const_iterator s_it;

	for (s_it = shipTypes.begin(); s_it != shipTypes.end(); ++s_it)
		shipPlacementPrompt(*s_it, display);
}

void BS_Board::placeShips()
{
	vector<Ship::shipInfo>::const_iterator s_it;

	for (s_it = shipTypes.begin(); s_it != shipTypes.end(); ++s_it)
		shipPlacementRandom(*s_it);
}

bool BS_Board::makeAttempt(char c, char r)
{
	int i = ((r - 1) * boardSize) + c - 'A';

	if (attackGrid[i].getHit()) {
		return false;
	} else {
		attackGrid[i].setHit();
		return true;
	}
}

bool BS_Board::checkShot(char c, char r, string& str)
{
	vector<Ship>::iterator s_it;

	str.clear();

	for (s_it = ships.begin(); s_it != ships.end(); ++s_it) {
		if ((*s_it).checkHit(c, r)) {
			str = (*s_it).type();
			int i = ((r - 1) * boardSize) + c - 'A';
			shipGrid[i].setHit();
			return true;
		}
	}
	return false;
}

void BS_Board::markHit(char c, char r)
{
	int i = ((r - 1) * boardSize) + c - 'A';
	attackGrid[i].setShip();

	/* Za AI
	if (strategy.valid()) {
		strategy.update(c, r);
	} else {
		strategy.set(c, r);
	}
	*/
}

bool BS_Board::fleetSunk()
{
	vector<Ship>::iterator s_it;
	for (s_it = ships.begin(); s_it != ships.end(); ++s_it) {
		if (!(*s_it).sunk()) {
			return false;
		}
	}
	return true;
}

int main()
{
    BattleShip bs(cin, cout);

	while(!bs.gameOver()) {
		bs.gameUpdate();
	}
	return 0;
}
