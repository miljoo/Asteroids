#include <iostream>
#include <fstream>      //tiedostonk‰sittelyyn vaadittava kirjasto file stream
#include <cmath>
#include <vector>       //vector s‰iliˆ, dynaaminen taulukko eli voidaan ohjelman ajon aikana kasvattaa ja pienent‰‰ kokoa tarpeen mukaan
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <pmmintrin.h>  //t‰‰lt‰ SSESqrt() funktion jutut, joilla saadan suoraan prosessorille k‰skytetty‰ toimintoja
                        //vaatii jonkin CPU architecture flagin projektiasetuksiin. esim -march=athlon64

#define PI 3.14159265359 //m‰‰ritell‰‰n PI niminen vakio. huom, t‰h‰n ei tule ;

//Globaalit muuttujat, joissa myˆhemmin luotavan ikkunan kokoarvot X ja Y suunnassa
int SCREENWIDTH = 1200;
int SCREENHEIGHT = 900;
bool DEBUG_MODE = 0;    //muuttuja, joka m‰‰ritt‰‰ onko itse tehty debug moodi p‰‰ll‰

using namespace std;  //https://www.cplusplus.com/doc/oldtutorial/namespaces/

enum{         //enumeroidaan nimettˆm‰sti muuttujat GAME,MAINMENU, HISCORE ja GAMEOVER
    GAME,     //k‰yt‰nnˆss‰ n‰m‰ sa arvot 0, 1, 2 ja 3 ja n‰ill‰ tullaan
    MAINMENU, //pit‰m‰‰n kirjaa ohjelman tilasta. Koodi jaetaan tilan mukaisiin lohkoihin
    HISCORE,
    GAMEOVER,
    NAMEINPUT
};

struct Score{ //rakenne, jossa pisteet ja pelaajan nimi n‰pp‰r‰sti yhdess‰
  int score;
  string name;
};

struct Hiscores{  //top10 pelaajien pisteiden s‰ilytt‰mist‰ varten oma rakenne
  Score scores[10]; //10 paikkainen taulukko Score rakennetta

  void saveHiscore(){ //tulosten tallentaminen tiedostoon hisc.ore
    ofstream file ("data/hisc.ore", ios::trunc); //trunc (truncate) asettaa tiedoston kirjoituksen kirjoittamaan vnahan datan p‰‰lle
    if(file.is_open()){                           //testataan onko tiedosto auki
      for(int i = 0 ; i < 10 ; i++){
        file << scores[i].name << endl;   //kirjoitetaan dataa tiedostoon sek‰ rivinvaihto
        file << scores[i].score << endl;
      }
      file.close();                       //suljetaan tiedosto sen j‰lkeen, kun sinne on tehty tarvittavat muutokset
    }
    else                                            //jos tiedostoa ei saada auki, tulostetaan virhe
    {
      cout << "Error saving hisc.ore file" << endl;
    }
  }

  void loadHiscore(){                 //sama kuin tallentaminen mutta pienill‰ muutoksilla
    ifstream file ("data/hisc.ore");
    if(file.is_open()){
      for(int i = 0 ; i < 10 ; i++){
        file >> scores[i].name;
        file >> scores[i].score;
      }
      file.close();
    }
    else
    {
      cout << "Error loading hisc.ore file" << endl;
    }
  }

  void resetHiscore(){              //pisteiden nollaus funktio
    for(int i = 0 ; i < 10 ; i++){
      scores[i].name = "AAA";
      scores[i].score = 0;
    }
    saveHiscore();                  //tallennetaan muutokset
  }

  void drawHiscores(sf::RenderWindow &window, sf::Font font, int index = -1){ //tulosten piirto, index k‰ytˆss‰ uuden huipputuloksen yhteydess‰
    string sScore;
    sf::Text text;
    text.setFont(font);
    text.setCharacterSize(48);
    text.setFillColor(sf::Color::Red);
    int padding = 20;

    for(int i = 0 ; i < 10 ; i++){
      sScore = to_string(scores[i].score);
      text.setString(sScore);
      text.setPosition(sf::Vector2f(SCREENWIDTH / 2 - 100,
                                    SCREENHEIGHT / 12 + i * (text.getCharacterSize() + padding)));

      if(i == index){
        text.setFillColor(sf::Color::Magenta);  //jos funktiolle annettu index arvo, niin sit‰ k‰ytet‰‰n muuttamaan tekstin v‰ri uuden syˆtetyn tuloksen kohdalla
      }
      else{
        text.setFillColor(sf::Color::Red);
      }
      window.draw(text);

      text.setString(scores[i].name);
      text.setPosition(sf::Vector2f(SCREENWIDTH / 2 + 50,
                                    SCREENHEIGHT / 12 + i * (text.getCharacterSize() + padding)));
      window.draw(text);
    }

    text.setString("Esc to return to mainmenu");
    text.setPosition(sf::Vector2f(SCREENWIDTH / 2 -200,
                                    SCREENHEIGHT / 12 + 10 * (text.getCharacterSize() + padding)));
    window.draw(text);
  }
};

struct Button{                //rakenne p‰‰valikon n‰pp‰imelle
  sf::RectangleShape button;
  sf::Vector2f buttonPos;
  sf::Vector2f buttonSize;
  sf::Color buttonColor;

  sf::FloatRect textOffSet;
  sf::Text text;
  sf::Font font;
  sf::Color textColor;
  string buttonTextStr;
  int fontSize;

  Button(){
    buttonPos = sf::Vector2f(0,0);
    buttonTextStr = "Default";
    font = sf::Font();
    fontSize = 10;
    textColor = sf::Color::Black;
    buttonSize = sf::Vector2f(100,100);
    buttonColor = sf::Color::White;

    text.setString(buttonTextStr);
    text.setFont(font);
    text.setCharacterSize(fontSize);
    text.setFillColor(textColor);

    button.setSize(buttonSize);
    button.setFillColor(buttonColor);
    button.setPosition(buttonPos);
  }

  Button(sf::Vector2f _pos, string _displayText, sf::Font _font, int _fontSize,
         sf::Color _textColor = sf::Color::Blue, sf::Vector2f _boxSize = sf::Vector2f(300.0f, 150.0f),
         sf::Color _boxColor = sf::Color::Green){

         buttonPos = _pos;
         buttonTextStr = _displayText;
         font = _font;
         fontSize = _fontSize;
         textColor = _textColor;
         buttonSize = _boxSize;
         buttonColor = _boxColor;

         text.setString(buttonTextStr);
         text.setFont(font);
         text.setCharacterSize(fontSize);
         text.setFillColor(textColor);

         button.setSize(buttonSize);
         button.setFillColor(buttonColor);
         button.setPosition(buttonPos);

         textOffSet = text.getLocalBounds();

         text.setOrigin(textOffSet.left + textOffSet.width / 2,
                        textOffSet.top + textOffSet.height / 2);
         text.setPosition(button.getPosition().x + button.getSize().x / 2,
                          button.getPosition().y + button.getSize().y / 2);
         }

         void updateColor(sf::Color color){
           buttonColor = color;
           button.setFillColor(buttonColor);
         }

         void draw(sf::RenderWindow &window){
            window.draw(button);
            window.draw(text);
         }
};

//Geneerinen rakenne pelielementeille, jossain isommassa peliss‰ t‰m‰ ei olisi hyv‰
//siit‰ syyst‰, ett‰ rakenteessa on turhaan muuttujia, joita kaikki pelin elementit
//eiv‰t tarvitse
struct Object{
    float x;          //s‰ilˆt‰‰n objektin x ja y sijainnit n‰iss‰ muuttujissa
    float y;
    float dx;         //dx ja dy on objektin x ja y suuntaisia kiihtyvyyksi‰ varten
    float dy;
    float angle;      //objektin kulma jos pyˆrii
    float osize;      //objektin koko
    double liveTime;  //objectin elossaoloaika
    bool alive;

    Object(){ //default constructor Object structin ilmentym‰lle esim Object player; niin pelaajalla alla m‰‰ritellyt arvot
        x = 0;
        y = 0;
        dx = 0;
        dy = 0;
        angle = 0;
        osize = 0;
        liveTime = 0;
        alive = true;
    }

    //parametrisoitu constructor, jossa lis‰ksi default arvoja jos kyseisi‰ arvoja ei luotaessa anneta
    Object(float _x, float _y, float _dx = 0, float _dy = 0, float _angle = 0, float _osize = 0, float _liveTime = 0, bool _alive = true)
    {
        x = _x;
        y = _y;
        dx = _dx;
        dy = _dy;
        angle = _angle;
        osize = _osize;
        liveTime = _liveTime;
        alive = _alive;
    }
};

struct BoundingBox{ //Laatikkorakenne, jota tullaan k‰ytt‰m‰‰n pelaajan collisioniin yms
    float x;          //laatikolle annetaan arvoina x ja y koordinaatit, vasen yl‰nurkka
    float y;          //sek‰ pituus ja leveys, joilla voidaan laskea kaikki muu laatikkoon liittyv‰
    float width;
    float height;
};

//Luodaan boundingbox, jolle lasketaan arvot, joista luotu laatikko pit‰‰ sis‰ll‰‰n kaikki vecModelin pisteet
BoundingBox createBBfromVecModel(const vector<pair<float, float> > &vecModelCoordinates ){
    BoundingBox result;
    int verts = vecModelCoordinates.size();
    float maxX = vecModelCoordinates[0].first;  //asetetaan suurimmiksi ja pienimmiksi x ja y arvoiksi ensimm‰inen piste
    float minX = vecModelCoordinates[0].first;
    float maxY = vecModelCoordinates[0].second;
    float minY = vecModelCoordinates[0].second;

    for(int i = 1 ; i < verts ; i++){              //verrataan muita pisteit‰ silmukassa ja p‰ivitet‰‰n suurimpia ja pienimpi‰ tarpeen mukaan
        if(vecModelCoordinates[i].first > maxX){
            maxX = vecModelCoordinates[i].first;
        }
        if(vecModelCoordinates[i].first < minX){
            minX = vecModelCoordinates[i].first;
        }

        if(vecModelCoordinates[i].second > maxY){
            maxY = vecModelCoordinates[i].second;
        }
        if(vecModelCoordinates[i].second < minY){
            minY = vecModelCoordinates[i].second;
        }
    }

    result.x = minX;
    result.y = minY;
    result.width = maxX - minX;
    result.height = maxY - minY;

    return result;
}

inline void SSESqrt(float * pOut, float * pIn){ //inline avainsanalla koodia k‰‰nnett‰ess‰ t‰m‰ koodi laitetaan paikkoihin, jossa funktiota kutsutaan
                                                //normaalissa funktiokutsussa koodin suoritus hypp‰‰ kutsuttavaan funktioon
  _mm_store_ss( pOut, _mm_sqrt_ss( _mm_load_ss(pIn))); //suoria prosessorikomentoja. https://www.intel.com/content/www/us/en/docs/intrinsics-guide/index.html
                                                        //projektiasetuksista tulee valita joku tietokonearkkitehtuuri, joka tukee n‰it‰, esim -march=athlon64
}

bool isPointInsideCircle(float cx, float cy, float radius, float px, float py ){ //palauttaa arvon, onko piste ympyr‰n sis‰ll‰
  float result; //syˆtetyt arvot ovat ympyr‰n keskipiste, s‰de, sek‰ pisteen sijainti
  float lenght = ((px-cx)*(px-cx) + (py-cy)*(py-cy)); //t‰ss‰ k‰yt‰nnˆss‰ potenssiin 2, mutta kertolaskuina, sill‰ s eon tehokkaampaa kuin pow() funktion k‰yttˆ
  SSESqrt(&result, &lenght);
  return result < radius; //palautetaan totuusarvo siit‰, onko pisteiden v‰linen et‰isyys pienempi kuin ympyr‰n s‰de
}

bool isPointInsideBox(int px,int py,int bx,int by, int bwidth, int bheight){    //onko piste laatikon sis‰ll‰
  if(px > bx && px < bx + bwidth && py > by && py < by + bheight){ return 1; }  //k‰ytet‰‰n esim kun tutkitaan onko hiiri p‰‰valikossa nappulan kohdala
  return 0;
}

bool isBBinsideCircle(BoundingBox bb, float cx, float cy, float radius){ //kertoo onko boundingbox laatikko ympyr‰n sis‰ll‰
    //ensin tarkistetaan laatikon kulmapisteet suhteessa ympyr‰‰n
    if(isPointInsideCircle(cx,cy,radius,bb.x,bb.y)){ return 1;}
    if(isPointInsideCircle(cx,cy,radius,bb.x + bb.width,bb.y)){ return 1;}
    if(isPointInsideCircle(cx,cy,radius,bb.x,bb.y + bb.height)){ return 1;}
    if(isPointInsideCircle(cx,cy,radius,bb.x + bb.width,bb.y + bb.height)){ return 1;}

    //sitten katsotaan kaksi eri suorakulmaista laatikkoa suhteessa pelaajan keskikohtaan.
    //t‰ss‰ siis ajatuksena Minkowski Addition https://en.wikipedia.org/wiki/Minkowski_addition
    //jossa ajatellaan, ett‰ pelaaja on piste (keskipiste) jonka tˆrm‰yst‰ verrataan muotoon, joka syntyy kun asteroidi
    //ja pelaaja muotoina summataan (ympyr‰ + suorakulmio)

    if(cx > bb.x &&
       cx < (bb.x + bb.width) &&
       cy > (bb.y - radius) &&
       cy < (bb.y + bb.height + radius)) { return 1;}

    if(cx > bb.x - radius &&
       cx < (bb.x + bb.width + radius) &&
       cy > bb.y &&
       cy < (bb.y + bb.height)) { return 1;}

    return 0;
}

//T‰ll‰ funktiolla siirret‰‰n ruudulta ulos menevi‰ asioita vastakkaiselle sivulle
//funktiolle annetaan parametreina sis‰‰n x ja y koordinaatit
//ja muuttujat, joihin kirjoitetaan mahdollisesti iffeiss‰ muuttuneet koordinaatit
//eli ox ja oy, joista kannattaa huomata erona ix ja iy:hyn '&' merkit
//eli n‰m‰ muuttujat v‰litet‰‰n suorana viitteen‰ muistipaikkoihin
//kun taas ix ja iy ovat kopioita syˆtetyist‰ arvoista

void wrapCoordinates(float ix, float iy, float& ox, float& oy){
  //sijoitetaan ulos kirjoitettaviin arvoihin sis‰‰n tulevat
  ox = ix;
  oy = iy;

  //jos pelaaja on ruudun ulkopuolla niin siirret‰‰n vastakkaiselle puolelle
  if(ix < 0.0f){
    ox = ix + (float)SCREENWIDTH;
  }
  if(ix > SCREENWIDTH){
    ox = ix - (float)SCREENWIDTH;
  }

  if(iy < 0.0f){
    oy = iy + (float)SCREENHEIGHT;
  }
  if(iy > SCREENHEIGHT){
    oy = iy - (float)SCREENHEIGHT;
  }
}

vector<pair<float,float> > transformWireFrameModel(const vector<pair<float,float> > &vecModelCoordinates,  //malli, joka piirret‰‰n
                        float x,              //x ja y koordinaatit, joihin lopputulos halutaan piirt‰‰
                        float y,
                        const float r = 0.0f, //kappaleen kulma, eli rotaatio
                        const float s = 1.0f) //kappaleen skaalaus, 1 t‰ss‰ defaulttina eli originaalin kokoinen
{
    vector<pair<float,float> > vecTransformedCoordinates;  //luodaan uusi vectori pair combo ja tehd‰‰n siit‰ funktiolle syˆtetyn version kanssa saman kokoinen

    int verts = vecModelCoordinates.size(); //otetaan verts muuttujaan tieto pisteiden (parien) lukum‰‰r‰st‰ vectorin size() funktiolla
    vecTransformedCoordinates.resize(verts);


    for(int i = 0 ; i < verts ; i++){   //Suoritetaan mallivectorin pisteille kiertoa r arvon mukaisesti ja tallennetaan yll‰ luotuun vectoriin
        vecTransformedCoordinates[i].first = vecModelCoordinates[i].first * cosf(r) - vecModelCoordinates[i].second * sinf(r);
        vecTransformedCoordinates[i].second = vecModelCoordinates[i].first * sinf(r) + vecModelCoordinates[i].second * cosf(r);
    }

    for(int i = 0 ; i < verts ; i++){             //t‰ss‰ suoritetaan skaalaus. *= on lyhyempi tapa esitt‰‰ seuraava koodirivi:
        vecTransformedCoordinates[i].first *= s;    //vecTransformedCoordinates[i].first = vecTransformedCoordinates[i].first * s;
        vecTransformedCoordinates[i].second *= s;   //samalla logiikalla voi myˆs tehd‰ +=, -= jne
    }

    for(int i = 0 ; i < verts ; i++){
        vecTransformedCoordinates[i].first += x;    //asetetaan muokatun mallin sijainti
        vecTransformedCoordinates[i].second += y;
    }
    return vecTransformedCoordinates;
}

//Funktio jolla piirret‰‰n vecModelCoordinates vectoriin syˆtetyn mallin mukainen muoto.
void drawWireFrameModel(sf::RenderWindow &window,                               //Ikkuna, johon piirret‰‰n
                        const vector<pair<float,float> > &vecModelCoordinates,  //malli, joka piirret‰‰n
                        sf::Color color = sf::Color::White) //default v‰ri valkoinen, jos ei t‰ss‰ kohtaan anna muuta v‰ri‰
{
    int verts = vecModelCoordinates.size();
    sf::ConvexShape convex;         //luodaa convex muoto, jolle asetetaan pistem‰‰r‰ksi vertsin sis‰lt‰m‰ luku
    convex.setPointCount(verts);

    for(int i = 0 ; i < verts ; i++){   //annetaan arvot convexin pisteille transformed vectorin muokattujen pisteiden mukaisesti
    convex.setPoint(i, sf::Vector2f(vecModelCoordinates[i % verts].first,
                                    vecModelCoordinates[i % verts].second));
    }

    //v‰ritet‰‰n muoto
    convex.setFillColor(color);
    //reunaviiva paksuuden asetus
    convex.setOutlineThickness(2);

    //lopulta piirret‰‰n muoto ruutubufferiin
    window.draw(convex);
}



//funktio jolla nollataan peli alkuarvoihinsa, suoritetaan pelin k‰ynnistyksess‰ ja jos pelaaja kuolee
void resetGame(Object &player, vector<Object> &asteroids, vector<Object> &bullets, int &score){
    player.x = SCREENWIDTH / 2;
    player.y = SCREENHEIGHT / 2;
    player.dx = 0;
    player.dy = 0;
    player.angle = 0;
    player.osize = 10;
    player.alive = true;

    score = 0;

    asteroids.clear();  //tyhjennet‰‰n asteroidit ja laitetaan kaksi aloitus asteroidia
    //N‰iden toimimiseksi tulisi Projectin Build optionseista k‰yd‰ laittamassa ruksi kohtaan "Have g++ follow the C++11 ISO..." compiler flagseista
    asteroids.push_back({(float)SCREENWIDTH * 0.2f, (float)SCREENHEIGHT * 0.2f, 40.0f, -10.0f, 0, 128, 0.0f, true }); //t‰ss‰ luodaan suoraan uusi Object hyv‰ksik‰ytt‰en
    asteroids.push_back({(float)SCREENWIDTH * 0.7f, (float)SCREENHEIGHT * 0.7f, -20.0f, 30.0f, 0, 128, 0.0f, true }); //c++11 mukana tuomaa { rakenne } syntaksia

    bullets.clear();
}

//funktio, jota voidaan k‰ytt‰‰ rajoittamaan annettu arvo annetun minini ja maksimi arvon v‰liin
void clamp(float &input, float MIN, float MAX){
    if(input < MIN){ input = MIN; }
    if(input > MAX){ input = MAX; }
}

int main()
{
  //luodaan ikkuna, ja sille kokoarvoina SCREENWIDTH ja SCREENHEIGHT sek‰ title "Asteroids"
  sf::RenderWindow window(sf::VideoMode(SCREENWIDTH, SCREENHEIGHT), "Asteroids");

  int state = MAINMENU; //asetetaan ohjelman tila p‰‰valikoksi

  sf::Vector2i mousePos;  //muuttuja hiiren sijaintia varten
  sf::Vector2i mousePressedPos; //t‰h‰n luetaan koordinaatit, jossa hiiren nappia painetaan

  bool spacePressed = false; //apumuuttuja v‰lilyˆnnill‰ ampumiseen, ett‰ saadaan 1 panos per painallus

  vector<Object> bullets;       //vectori panoksien datan s‰ilˆnt‰‰n
  float bulletSpeed = 400.0f;   //panosten ampumisnopeus, peliss‰ nopeuteen lis‰t‰‰n myˆs pelaajan liike
  int maxBullets = 5;           //maksimi m‰‰r‰ panoksia peliss‰ kerrallaan

  vector<Object> newAsteroids;
  vector<Object> asteroids;

  Object player;  //Luodaan yll‰olevan Object structin mukainen ilmentym‰ (olio) nimelt‰ player, jolla siis on structissa m‰‰ritellyt j‰senmuuttuja

  Hiscores hs;    //Luodaan Hiscores rakenteesta ilmentym‰ hs. T‰ss‰ kohtaa siis varataan muistista structissa m‰‰ritellyn datan verran ja hs ilmentym‰n kautta voi k‰ytt‰‰ rakenteen funktioita

  int score = 0;
  int scoreIndex = -1;
  string scoreStr;
  string nameInput = "";
  string insertNamePrompt = "Insert your name.";
  bool isTyping = false;

  resetGame(player, asteroids, bullets, score);

  BoundingBox playerBB;

  float asteroidMaxSpeed = 150.0f;
  float asteroidSpinSpeed = 0.5f;
  float asteroidSpeedOnRespawn = 50.0f;

  float playerMaxSpeed = 500;
  float playerMovespeed = 2.0f;  //Arvo, jota k‰ytet‰‰n pelaajan liikkumiseen
  float playerTurnspeed = 4.0f;   //Arvo, jota k‰ytet‰‰n pelaajan k‰‰ntymiseen

  /*Luodaan vector, joka on tyyppi‰ pair, joka on tyyppi‰ float ja float eli pari, jonka kummatkin
  elementit ovat float tyyppisi‰. Vector on cpp:n mukana tuleva dynaaminen s‰iliˆ, joka toiminnaltaan
  muistuttaa taulukkoa, mutta voi ohjelman ajon aikana kasvaa ja pienet‰ tarpeen mukaan
  */
  vector<pair<float, float> > vecModelShip;
  vecModelShip.push_back(make_pair(0, -3)); //puch_back funktiolla vectoriin lis‰t‰‰n elementtej‰
  vecModelShip.push_back(make_pair(3, 2));  //make_pair funktio muodostaa pair s‰iliˆˆn sis‰llˆksi
  vecModelShip.push_back(make_pair(0, 1));  //annmainmenuetut arvot. Arvot muodostavat X/Y koordinaatistossa pelaajan
  vecModelShip.push_back(make_pair(-3, 2)); //aluksen muotoisen muodon, jos pisteit‰ yhdistelt‰isiin viivoilla
  vector<pair<float, float> > vecShip;

  vector<pair<float, float> > vecModelAsteroid;   //Asteroideille oma vectori
  int verts = 20;                                 //tehd‰‰n 20 pisteest‰ koostuva ympyr‰...
  for(int i = 0 ; i < verts ; i++){
    float noise = (float)rand() / (float)RAND_MAX * 0.4f + 0.8f;  //... johon noise muuttujalla tehd‰‰n rˆpelˆisyytt‰
    vecModelAsteroid.push_back(make_pair(noise * sinf(((float)i / (float)verts) * 2 * PI),
                                         noise * cosf(((float)i / (float)verts) * 2 * PI)));
  }

  sf::Time dtime;         //t‰h‰n muuttujaan lasketaan delta aika, eli kauan kuluu yhden framen suoritukseen
  sf::Time previousTime;  //pari apumuuttujaa dtimen laskemista varten
  sf::Time elapsedTime;
  sf::Clock clock;        //kello josta saadaan aika-arvoja

  //Ladataan ‰‰nitiedostoja data kansiosta.
  sf::SoundBuffer buffer;
  if(!buffer.loadFromFile("data/shoot.wav")){
    cout << "Error loading shoot.wav" << endl;
  }
  sf::Sound shootFX;
  shootFX.setBuffer(buffer);

   sf::SoundBuffer buffer2;
  if(!buffer2.loadFromFile("data/asteroidHit.wav")){
    cout << "Error loading asteroidHit.wav" << endl;
  }
  sf::Sound asteroidHitFX;
  asteroidHitFX.setBuffer(buffer2);

   sf::SoundBuffer buffer3;
  if(!buffer3.loadFromFile("data/playerDeath.wav")){
    cout << "Error loading playerDeath.wav" << endl;
  }
  sf::Sound playerDeathFX;
  playerDeathFX.setBuffer(buffer3);

  sf::Music music;
  if (!music.openFromFile("data/music.ogg"))
      return -1; // error
  music.play();

  music.setVolume(50);

  sf::Texture texture;    //Luodaan textuuri rakenne, johon iffin sis‰ll‰ yritet‰‰n ladata tiedostoa, joka sijaitsee projektikansiossa
  if(!texture.loadFromFile("data/panos.png")){ //sijaitsevassa data kansiossa. loadFromFile palauttaa latauksen onnistuessa arvon 1
    cout << "Error loading panos.png" << endl; //joten if ehdon alussa oleva ! muuttaa sen 0, eli falseksi, jolloin iffi‰ ei suoriteta
  }                                            //jos kuvan lataus onnistuu. Kuva siis ladataan if ehdossa

  sf::Sprite spr_bullet;          //luodaan sprite, johon asetetaan grafiikaksi yll‰ ladattu tekstuuri
  spr_bullet.setTexture(texture);
  spr_bullet.setOrigin(texture.getSize().x / 2, texture.getSize().y / 2); //asetetaan kuvan origin, eli keskipiste, sijaitsemaan kuvan keskelle

  sf::Font font;
  if(!font.loadFromFile("data/font.TTF")){  //ladataan teksti‰ varten fonttitiedosto
    cout << "Error loading font.TTF" << endl; //fonttitiedosto kopioitiin c:\windows\fonts kansiosta ja nimettiin font.TFF
  }

  sf::Text text;    //luodaan texti muuttuja ja asetetaan sille ladattu fontti
  text.setFont(font);
  text.setString("Testi");
  text.setCharacterSize(48);
  text.setFillColor(sf::Color::Red);

  //nappuloihin liittyvi‰ muuttujia
  float padding = 100.0f;
  float buttonWidth = 300.0f;
  float buttonHeight = 100.0f;

  vector<Button> mainmenu;

  Button play = {sf::Vector2f(SCREENWIDTH / 2 - buttonWidth / 2 , SCREENHEIGHT / 2 - buttonHeight - padding),
                "Play", font, 48, sf::Color::Red, sf::Vector2f(buttonWidth, buttonHeight), sf::Color::Green};
  Button hiscore = {sf::Vector2f(SCREENWIDTH / 2 - buttonWidth / 2 , SCREENHEIGHT / 2 - buttonHeight),
                "Hiscore", font, 48, sf::Color::Red, sf::Vector2f(buttonWidth, buttonHeight), sf::Color::Green};
  Button exit = {sf::Vector2f(SCREENWIDTH / 2 - buttonWidth / 2 , SCREENHEIGHT / 2 - buttonHeight + padding),
                "Exit", font, 48, sf::Color::Red, sf::Vector2f(buttonWidth, buttonHeight), sf::Color::Green};

  mainmenu.push_back(play);
  mainmenu.push_back(hiscore);
  mainmenu.push_back(exit);

  //T‰st‰ alkaa ohjelmasilmukka
  while (window.isOpen())
  {
    dtime = elapsedTime - previousTime;   //delta time saadaan laskemalla kulunut aika - edellinen aika
    previousTime = clock.getElapsedTime();//t‰ss‰ otetaan ohjelman k‰ynnistyksest‰ alkanut ajanhetki

    window.clear();     //tyhjennet‰‰n piirtobufferi

    sf::Event event;   //event muuttuja tapahtumien k‰sittely‰ varten
    while (window.pollEvent(event)) //while silmukka, jonka sis‰ll‰ puretaan kertyneet tapahtumat
    {
        if(event.type == sf::Event::Closed){
            window.close();
        }

        if(event.type == sf::Event::KeyPressed){

            if(event.key.code == sf::Keyboard::D){  //d nappulalla voidaan vaihdella DEBUG_MODE:n arvoa v‰lill‰ 0 ja 1, eli false ja true
              if(state == GAME){
                DEBUG_MODE = !DEBUG_MODE;            //t‰ss‰ k‰ytet‰‰n hyv‰ksi ! eli negaatiota
              }
            }
            if(event.key.code == sf::Keyboard::Escape){   //jos painetaan Esc n‰pp‰int‰...
                if(state == GAME || state == HISCORE){    //jos ohjelman tila on GAME TAI HISCORE
                  state = MAINMENU;                       //asetetaan tilaksi MAINMENU, eli palataan p‰‰valikkoon
                }
                else if (state == MAINMENU){              //muuttatapauksessa jos tila on MAINMENU
                    window.close();                       //suljetaan ikkuna/ohjelma
                }
            }

            if(event.key.code == sf::Keyboard::M){  //M nappulalla laitetaan musiikkia pois/p‰‰lle. Alla linkki SFML dokumentaatioon, jossa lis‰ tietoa
              if(music.getStatus() == sf::SoundSource::Status::Playing){ //https://www.sfml-dev.org/documentation/2.5.1/classsf_1_1SoundSource.php#ac43af72c98c077500b239bc75b812f03
                music.pause();
              }
              else
              {
                music.play();
              }
            }
        }
        if(state == NAMEINPUT){
          if(event.type == sf::Event::TextEntered){
            if(event.text.unicode < 128){
              if(event.text.unicode == 13){
                isTyping = false;
              }
              else if(event.text.unicode == 8){
                if(nameInput.size() > 0){
                  nameInput.resize(nameInput.size() - 1);
                }
              }
              else
              {
                nameInput += static_cast<char>(event.text.unicode);
              }
            }
          }
        }

    }

    //*******************************************
    //INPUT osio
    //*******************************************
    if(state == GAME){
      if(sf::Keyboard::isKeyPressed(sf::Keyboard::Left)){    //jos painetaan vasenta nuolinappulaa
        player.angle -= playerTurnspeed * dtime.asSeconds(); //k‰‰nnet‰‰n pelaajaa vasemmalle
      //T‰m‰ alla oleva kommentoitu rivi, sek‰ muiden suuntien iffeiss‰ olevat kommentoidut
      //rivit ovat esimerkkin‰ siit‰, miten saada pelaaja liikkumaan painallusten suuntaan
      //eik‰ niit‰ k‰ytet‰ t‰ss‰ peliss‰, ovat siis vaan esimerkkin‰ jos haluaa ite tehd‰ jotain
      //erilaista peli‰ k‰ytt‰en t‰t‰ pohjana tai esimerkkin‰

      //player.x -= playerMovespeed * dtime.asSeconds();
      }

      if(sf::Keyboard::isKeyPressed(sf::Keyboard::Right)){   //ja sama juttu oikealle
        player.angle += playerTurnspeed * dtime.asSeconds();
        //player.x += playerMovespeed * dtime.asSeconds();
      }

      if(sf::Keyboard::isKeyPressed(sf::Keyboard::Up)){
        player.dx += sin(player.angle) * playerMovespeed;// * dtime.asSeconds();
        player.dy += -cos(player.angle) * playerMovespeed;// * dtime.asSeconds();
        //player.y -= playerMovespeed * dtime.asSeconds();text.setPosition(sf::Vector2f(SCREENWIDTH / 2 + 50,
      }

      if(sf::Keyboard::isKeyPressed(sf::Keyboard::Down)){
        //player.y += playerMovespeed * dtime.asSeconds();
      }

      if(sf::Keyboard::isKeyPressed(sf::Keyboard::Space)){
        if(!spacePressed){  //t‰ll‰ if ehdolla estet‰‰n ampuminen enemm‰n kuin kerran per v‰lilyˆnnin painallus
          if(bullets.size() < maxBullets){ //katotaan onko maksimi m‰‰r‰ panoksia jo saavutettu
            shootFX.play();
            //jos ei niin lis‰t‰‰n bullets vectoriin panos, joka l‰htee pelaajan keskikohdasta pelaajan osoittamaan suuntaan ja vaihti mukaan huomioituna
            bullets.push_back({player.x, player.y, bulletSpeed * sinf(player.angle) + player.dx,
                                                -bulletSpeed * cosf(player.angle) + player.dy,
                                                0, 0, 100.0f, true});
          }
          spacePressed = true;  //kun v‰lilyˆnti on painettu, asetetaan t‰m‰ arvo todeksi. arvo muuttuu ep‰todeksi else kohdassa vasta kun v‰lilyˆnti‰ ei en‰‰ paineta
                                //t‰m‰ siksi, ett‰ yhden painalluksen aikana koodia suoritetaan useita kertoja, jolloin v‰lilyˆntipainallus ehto toteutuu ja n‰m‰ suoritetaan
        }
      }
      else
      {
          if(spacePressed){
              spacePressed = false;
          }
      }
    }

    if(state == MAINMENU)  //t‰st‰ alkaa p‰‰valikkolohko
    {
      if(scoreIndex != -1){
         scoreIndex = -1;
      }

      mousePos = sf::Mouse::getPosition(window);

      if(sf::Mouse::isButtonPressed(sf::Mouse::Left)){
        mousePressedPos = sf::Mouse::getPosition(window);
      }

      for(unsigned int i = 0 ; i < mainmenu.size() ; i++){
        mainmenu[i].draw(window);

        if(isPointInsideBox(mousePos.x , mousePos.y ,
                            mainmenu[i].buttonPos.x , mainmenu[i].buttonPos.y ,
                            mainmenu[i].buttonSize.x, mainmenu[i].buttonSize.y))
        {
            mainmenu[i].updateColor(sf::Color::Cyan);
        }
        else
        {
            if(mainmenu[i].buttonColor != sf::Color::Blue){
              mainmenu[i].updateColor(sf::Color::Blue);
            }

        }

        if(isPointInsideBox(mousePressedPos.x , mousePressedPos.y ,         //tutkitaan onko hiirel‰ painettu jotakin nappulaa
                            mainmenu[i].buttonPos.x , mainmenu[i].buttonPos.y ,
                            mainmenu[i].buttonSize.x, mainmenu[i].buttonSize.y))
        {
          switch(i){          //jos on niin i sis‰lt‰‰ nappulan indeksin ja switch case rakenteessa tutkitaan mik‰ se oli
          case 0:             //jos on ikseksi oli 0, eli mainmenu:n ensimm‰inen nappula (t‰ss‰ tapauksessa play) siirryt‰‰n pelitilaan
            state = GAME;
            break;
          case 1:
            state = HISCORE;
            break;
          case 2:
            window.close();
            break;
          }
          mousePressedPos.x = -1; //lopuksi "nollataan" hiiren klikkaus t‰ss‰ ehk‰ v‰h‰n rumasti. joku boolean voisi olla n‰timpi tapa
          mousePressedPos.y = -1;
        }
      }
    }
    else if(state == NAMEINPUT){
      if(hs.scores[scoreIndex].score != score){
        hs.scores[scoreIndex].score = score;
      }

      //window.clear();
      text.setString(insertNamePrompt);
      text.setPosition(sf::Vector2f(SCREENWIDTH/2 - 300, SCREENHEIGHT / 5));
      window.draw(text);

      text.setString(nameInput);
      text.setPosition(sf::Vector2f(SCREENWIDTH/2 - 300, SCREENHEIGHT / 2));
      window.draw(text);
      //window.display();
      if(!isTyping){
        hs.scores[scoreIndex].name = nameInput;
        hs.saveHiscore();

        nameInput = "";
        resetGame(player, asteroids, bullets, score);
        state = HISCORE;
      }
    }
    else if(state == HISCORE)//t‰ss‰ hiscore lohko
    {
      hs.loadHiscore();
      hs.drawHiscores(window, font, scoreIndex);
    }
    else if(state == GAME)  //t‰ss‰ pelilohko
    {
      text.setPosition(sf::Vector2f(0, 0));
      //Pelaajan liikkumiset yms
      if(player.angle >= 2 * PI){player.angle = 0;} //pidet‰‰n kulman arvot v‰lill‰ 0-2 * PI
      if(player.angle < 0){player.angle = 2 * PI;}  //koska kulman arvot ovat radiaaneissa, eik‰ asteissa. https://fi.wikipedia.org/wiki/Radiaani

      clamp(player.dx , -playerMaxSpeed, playerMaxSpeed);
      clamp(player.dy , -playerMaxSpeed, playerMaxSpeed);

      player.x += player.dx * dtime.asSeconds(); //pelaajan sijainti x suunnassa
      player.y += player.dy * dtime.asSeconds(); //pelaajan sijainti y suunnassa
      wrapCoordinates(player.x, player.y, player.x, player.y);  //syˆtet‰‰n funktiolle pelaajan sijainti ja tarpeen mukaan tapahtuu wrap

      //Panos jutut
      for(unsigned int i = 0 ; i < bullets.size() ; i++){  //panoksille tehd‰‰n samoja juttuja kuin asteroideille
          bullets[i].x += bullets[i].dx * dtime.asSeconds();
          bullets[i].y += bullets[i].dy * dtime.asSeconds();
          wrapCoordinates(bullets[i].x, bullets[i].y, bullets[i].x, bullets[i].y);

          for(unsigned int j = 0 ; j < asteroids.size() ; j++){  //sen lis‰ksi t‰‰ll‰ myˆs tarkastellaan jokaisen panoksen osumista jokaiseen asteroidiin
            if(isPointInsideCircle(asteroids[j].x, asteroids[j].y, asteroids[j].osize, //t‰‰ll‰ siis huomattava, ett‰ panokset ovat i indeksill‰ ja asteroidit j
                                   bullets[i].x, bullets[i].y))
            {
              asteroidHitFX.play();
              if(asteroids[j].osize > 8){
                  float asteroidSpeed = 4 * asteroids[j].dx;
                  if(asteroidSpeed > asteroidMaxSpeed){ asteroidSpeed = asteroidMaxSpeed; }

                  float angle1 = ( (float)rand() / (float)RAND_MAX ) * 2 * PI;
                  float angle2 = ( (float)rand() / (float)RAND_MAX ) * 2 * PI;

                  newAsteroids.push_back({asteroids[j].x, asteroids[j].y, asteroidSpeed * sinf(angle1), asteroidSpeed * cosf(angle1), 0, (int)asteroids[j].osize >> 1, 0.0f, true });
                  newAsteroids.push_back({asteroids[j].x, asteroids[j].y, asteroidSpeed * sinf(angle2), asteroidSpeed * cosf(angle2), 0, (int)asteroids[j].osize >> 1, 0.0f, true });
              }

              bullets[i].alive = false;    //panosten ja asteroidien pelist‰ poistaminen tapahtuu pienen kiertoreitin kautta asettamalla niiden alive tila falseksi
              asteroids[j].alive = false;  //niit‰ EI saa poistaa t‰‰ll‰ silmukan sis‰ll‰, sill‰ hommat hajoaa jos menn‰‰n poistamaan vectorista kesken sen l‰pik‰ynnin elementtej‰
              score += 100;
            }
          }
        //TODO: siirr‰ n‰‰ t‰st‰ muualle, koska panokset saattaa poistua pelist‰ ennen t‰t‰ kohtaa
          spr_bullet.setPosition( sf::Vector2f(bullets[i].x, bullets[i].y) );
          window.draw(spr_bullet);
      }

      if(newAsteroids.size() > 0){
          for(unsigned int i = 0 ; i < newAsteroids.size() ; i++){
              asteroids.push_back(newAsteroids[i]);
          }
          newAsteroids.clear();
      }

      //Poistetaan tuhotut asteroidit ja panokset
      if(asteroids.size() > 0){
        //jos n‰iden t‰sm‰llinen toiminta kiinnostaa tarkemmin, niin googleen t‰ss‰ esiintyvi‰ asioita ja itse selvittelem‰‰n
        auto i = remove_if(asteroids.begin(), asteroids.end(), [&](Object o){ return (!o.alive); } );

        if(i != asteroids.end()){ //lyhyesti sanottuna remove_if j‰rjest‰‰ vecotin sis‰llˆn sitne, ett‰ asiat jotka ovat x akselilla pienemm‰t kuin 0
          asteroids.erase(i);     //tulevat sitten t‰ss‰ erasessa poistetuksi
        }
      }

      if(bullets.size() > 0){
        auto i = remove_if(bullets.begin(), bullets.end(), [&](Object o){ return (!o.alive); } );
        if(i != bullets.end()){
          bullets.erase(i);
        }
      }

      if(asteroids.empty()){
          score += 1000;
          asteroids.clear();
          bullets.clear();

          asteroids.push_back({300.0f * sinf(player.angle - PI/2.0f) + player.x,
                              300.0f * cosf(player.angle - PI/2.0f) + player.y,
                              asteroidSpeedOnRespawn * sinf(player.angle),
                              asteroidSpeedOnRespawn * cosf(player.angle),
                              0, 128, 0.0f, true });

          asteroids.push_back({300.0f * sinf(player.angle + PI/2.0f) + player.x,
                              300.0f * cosf(player.angle + PI/2.0f) + player.y,
                              asteroidSpeedOnRespawn * sinf(-player.angle),
                              asteroidSpeedOnRespawn * cosf(-player.angle),
                              0, 128, 0.0f, true });
      }

      //Asteroidien jutut
      for(unsigned int i = 0 ; i < asteroids.size() ; i++){            //k‰yd‰‰n for loopissa kaikki asteroidit l‰pi k‰ytt‰en hyv‰ksi vectorin size() funktiota

            asteroids[i].x += asteroids[i].dx * dtime.asSeconds();//m‰‰rittelem‰‰n for loopin lopetus arvon
            asteroids[i].y += asteroids[i].dy * dtime.asSeconds();//tehd‰‰n jokaiselle asteroidille sijainti yms p‰ivitykset
            asteroids[i].angle += asteroidSpinSpeed * dtime.asSeconds();

            wrapCoordinates(asteroids[i].x, asteroids[i].y, asteroids[i].x, asteroids[i].y); //asteroidit myˆs menee ruudun laidasta toisele

            drawWireFrameModel(window, transformWireFrameModel(vecModelAsteroid, asteroids[i].x, asteroids[i].y, asteroids[i].angle, asteroids[i].osize), sf::Color::Red); //piirret‰‰n asteroidi

            if(DEBUG_MODE){
                sf::CircleShape asteroidDEBUG(asteroids[i].osize);
                asteroidDEBUG.setFillColor(sf::Color::Yellow);
                asteroidDEBUG.setOutlineColor(sf::Color::Red);
                asteroidDEBUG.setOutlineThickness(5);
                asteroidDEBUG.setPosition(asteroids[i].x - asteroids[i].osize , asteroids[i].y - asteroids[i].osize);
                window.draw(asteroidDEBUG);
            }
      }

      vecShip = transformWireFrameModel(vecModelShip, player.x, player.y, player.angle, player.osize);
      playerBB = createBBfromVecModel(vecShip);

      if(DEBUG_MODE){
          sf::RectangleShape playerBB_DEBUG;
          playerBB_DEBUG.setFillColor(sf::Color::Transparent);
          playerBB_DEBUG.setSize(sf::Vector2f(playerBB.width, playerBB.height ));
          playerBB_DEBUG.setPosition(playerBB.x , playerBB.y);
          playerBB_DEBUG.setOutlineColor(sf::Color::Red);
          playerBB_DEBUG.setOutlineThickness(2);
          window.draw(playerBB_DEBUG);
      }

      drawWireFrameModel(window, vecShip, sf::Color::Blue); //piirret‰‰n pelaaja

      for(unsigned int i = 0 ; i < asteroids.size() ; i++){
          if(isBBinsideCircle(playerBB, asteroids[i].x , asteroids[i].y, asteroids[i].osize)){
              playerDeathFX.play();
              state = GAMEOVER;
          }
      }

      scoreStr = to_string(score);
      text.setString(scoreStr);
      window.draw(text);
    }
    else if(state == GAMEOVER)
    {
      scoreIndex = -1;
      hs.loadHiscore();

      for(int i = 0 ; i < 10 ; i++){
        if(score > hs.scores[i].score){
          scoreIndex = i;
          break;
        }
      }

      if(scoreIndex >= 0){
        for(int i = 8; i >= scoreIndex ; i--){
          hs.scores[i + 1].name = hs.scores[i].name;
          hs.scores[i + 1].score = hs.scores[i].score;
        }
        isTyping = true;
        state = NAMEINPUT;
      }

      if(state != NAMEINPUT){
        resetGame(player, asteroids, bullets, score);
        state = HISCORE;
      }
    }

    window.display();   //siirret‰‰n piirtobufferi ruudulle
    elapsedTime = clock.getElapsedTime(); //otetaan talteen framen lopussa oleva aika
  }
  return 0;
}
