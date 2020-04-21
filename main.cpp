#include <iostream>
#include <fstream>
#include <cmath>
#include <vector>
#include <algorithm>
#include <string>
#include <pmmintrin.h>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

#define PI 3.14159265

#define DEBUG_MODE 0

int ScreenWidth = 1900;
int ScreenHeight = 800;

using namespace std;

struct BoundingBox{
    float x;
    float y;
    float width;
    float height;
};

struct TextBox{
    sf::RectangleShape rect;
    string text;
};

struct Button{
    sf::RectangleShape rect;
    sf::Text text;
    sf::Vector2f pos;
    sf::Font font;
    sf::Color text_color;
    sf::Vector2f box_size;
    sf::Color box_color;
    sf::FloatRect textOffset;
    int font_size;
    string display_text;

    Button();
    Button(sf::Vector2f _pos,
           string _display_text,
           sf::Font _font,
           int _font_size,
           sf::Color _text_color = sf::Color::Red,
           sf::Vector2f _box_size = sf::Vector2f(300.0f, 150.0f),
           sf::Color _box_color = sf::Color::Blue)
    {
        pos = _pos;
        display_text = _display_text;
        font = _font;
        font_size = _font_size;
        text_color = _text_color;
        box_size = _box_size;
        box_color = _box_color;

        text.setString(display_text);
        text.setFont(font);
        text.setCharacterSize(font_size);
        text.setFillColor(text_color);

        rect.setSize(box_size);
        rect.setFillColor(box_color);
        rect.setPosition(pos);

        textOffset = text.getLocalBounds();
        text.setOrigin(textOffset.left + textOffset.width/2.0f, textOffset.top  + textOffset.height/2.0f);
        text.setPosition(rect.getPosition().x + rect.getSize().x / 2, rect.getPosition().y + rect.getSize().y / 2);
    }

    void update()
    {
        text.setString(display_text);
        text.setFont(font);
        text.setCharacterSize(font_size);
        text.setFillColor(text_color);

        rect.setSize(box_size);
        rect.setFillColor(box_color);
        rect.setPosition(pos);

        textOffset = text.getLocalBounds();
        text.setOrigin(textOffset.left + textOffset.width/2.0f, textOffset.top  + textOffset.height/2.0f);
        text.setPosition(rect.getPosition().x + rect.getSize().x / 2, rect.getPosition().y + rect.getSize().y / 2);
    }

    void draw(sf::RenderWindow &window)
    {
        window.draw(rect);
        window.draw(text);
    }
};

struct Object{
    int osize;
	float x;
	float y;
	float dx;
	float dy;
	float angle;
	float liveTime;
};

struct Score{
    int score;
    string name;
};

struct Highscores{
    Score scores[10];

    void load_highscores(){
        string line;
        ifstream file ("data/hisc.ore");
        if (file.is_open())
        {
           for(int i = 0 ; i < 10 ; i++)
            {
                file >> scores[i].name;
                file >> scores[i].score;
            }
            file.close();
        }
        else cout << "Unable to load file";
    }

    void save_highscores(){
        ofstream file ("data/hisc.ore", ios::trunc);
        if (file.is_open())
        {

            for(int i = 0 ; i < 10 ; i++)
            {
                file << scores[i].name << endl;
                file << scores[i].score << endl;
            }

            file.close();
        }else cout << "Unable to save file";
    }

    void print_highscores(sf::RenderWindow &window, sf::Font font, int index = -1)
    {
        string s_score;
        sf::Text text;
        text.setFont(font);
        text.setCharacterSize(48);
        text.setFillColor(sf::Color::Red);
        int padding = 20;

        window.clear();
        for(int i = 0 ; i < 10 ; i++)
        {
            s_score = to_string(scores[i].score);
            text.setString(s_score);
            text.setPosition(sf::Vector2f(ScreenWidth / 2 - 100, ScreenHeight / 12 + i * (text.getCharacterSize() + padding)));

            if(i == index)
            {
                text.setFillColor(sf::Color::Magenta);
            }
            else
            {
                text.setFillColor(sf::Color::Red);
            }

            window.draw(text);

            text.setString(scores[i].name);
            text.setPosition(sf::Vector2f(ScreenWidth / 2 + 50, ScreenHeight / 12 + i * (text.getCharacterSize() + padding)));
            window.draw(text);
        }
        text.setString("Press enter to continue");
        text.setPosition(sf::Vector2f(ScreenWidth / 2 - 200, ScreenHeight / 12 + 10 * (text.getCharacterSize() + padding)));
        window.draw(text);
    }

    void reset_highscore()
    {
        for(int i = 0 ; i < 10 ; i++){
            scores[i].name = "Foo";
            scores[i].score = 0;
        }
    }
};

BoundingBox createBBfromVecModel(const vector<pair<float, float> > &vecModelCoordinates){
    BoundingBox result;
    float offset = 10.0f;
    int verts = vecModelCoordinates.size();
    float maxX = vecModelCoordinates[0].first;
    float minX = vecModelCoordinates[0].first;
    float maxY = vecModelCoordinates[0].second;
    float minY = vecModelCoordinates[0].second;

    for(int i = 1 ; i < verts ; i++){
        if(vecModelCoordinates[i].first > maxX){
            maxX = vecModelCoordinates[i].first - offset;
        }
        if(vecModelCoordinates[i].first < minX){
            minX = vecModelCoordinates[i].first + offset;
        }
        if(vecModelCoordinates[i].second > maxY){
            maxY = vecModelCoordinates[i].second - offset;
        }
        if(vecModelCoordinates[i].second < minY){
            minY = vecModelCoordinates[i].second + offset;
        }
    }

    result.x = minX;
    result.y = minY;
    result.width = maxX - minX;
    result.height = maxY - minY;

    return result;
}

void wrapCoordinates(float ix, float iy, float &ox, float &oy)
{
	ox = ix;
	oy = iy;

	if(ix < 0.0f){ox = ix + (float)ScreenWidth;}
	if(ix >= ScreenWidth){ox = ix - (float)ScreenWidth;}

	if(iy < 0.0f){oy = iy + (float)ScreenHeight;}
	if(iy >= ScreenHeight){oy = iy - (float)ScreenHeight;}
}

inline void SSESqrt( float * pOut, float * pIn )
{
   _mm_store_ss( pOut, _mm_sqrt_ss( _mm_load_ss( pIn ) ) );
}

bool isPointInsideCircle(float cx, float cy, float radius, float x, float y)
{
    float result;
    float length = ((x-cx)*(x-cx) + (y-cy)*(y-cy));
    SSESqrt(&result, &length);
    return result < radius;
}

bool isBBInsideCircle(BoundingBox bb, float cx, float cy, float radius){
    //Minkowski sum
    if(isPointInsideCircle(bb.x, bb.y, radius, cx, cy)){return 1;}
    if(isPointInsideCircle(bb.x, bb.y + bb.height, radius, cx, cy)){return 1;}
    if(isPointInsideCircle(bb.x + bb.width, bb.y, radius, cx, cy)){return 1;}
    if(isPointInsideCircle(bb.x + bb.width, bb.y + bb.height, radius, cx, cy)){return 1;}

    if(cx > bb.x &&
       cx < (bb.x + bb.width) &&
       cy > (bb.y - radius) &&
       cy < (bb.y + bb.height + radius))
    {
        return 1;
    }

    if(cx > (bb.x - radius) &&
       cx < (bb.x + bb.width + radius) &&
       cy > bb.y &&
       cy < (bb.y + bb.height ))
    {
        return 1;
    }
    return 0;
}

void drawWireFrameModel(sf::RenderWindow &window,
                        const vector<pair<float, float> > &vecModelCoordinates,
                        float x,
                        float y,
                        const float r = 0.0f,
                        const float s = 1.0f,
                        sf::Color col = sf::Color::White)
{
    // Create translated model vector of coordinate pairs
    vector<pair<float, float> > vecTransformedCoordinates;
    int verts = vecModelCoordinates.size();
    vecTransformedCoordinates.resize(verts);

    // Rotate
    for (int i = 0; i < verts; i++)
    {
        vecTransformedCoordinates[i].first = vecModelCoordinates[i].first * cosf(r) - vecModelCoordinates[i].second * sinf(r);
        vecTransformedCoordinates[i].second = vecModelCoordinates[i].first * sinf(r) + vecModelCoordinates[i].second * cosf(r);
    }

    // Scale
    for (int i = 0; i < verts; i++)
    {
        vecTransformedCoordinates[i].first = vecTransformedCoordinates[i].first * s;
        vecTransformedCoordinates[i].second = vecTransformedCoordinates[i].second * s;
    }

    // Translate
    for (int i = 0; i < verts; i++)
    {
        vecTransformedCoordinates[i].first = vecTransformedCoordinates[i].first + x;
        vecTransformedCoordinates[i].second = vecTransformedCoordinates[i].second + y;
    }

    // Draw Closed Polygon

    // create an empty shape
    sf::ConvexShape convex;

    // resize it to verts points
    convex.setPointCount(verts);

    // define the points
     for (int i = 0; i < verts; i++)
    {
        convex.setPoint(i, sf::Vector2f(vecTransformedCoordinates[i % verts].first, vecTransformedCoordinates[i % verts].second));
    }
    convex.setFillColor(col);
    convex.setOutlineThickness(2);

    window.draw(convex);

    if(DEBUG_MODE)
    {
        sf::CircleShape as_debug(s);
        as_debug.setFillColor(sf::Color::Yellow);
        as_debug.setPosition(x - s, y - s);
        as_debug.setOutlineColor(sf::Color::Red);
        as_debug.setOutlineThickness(5);
        window.draw(as_debug);
    }



}

void drawWireFrameModel(sf::RenderWindow &window,
                        const vector<pair<float, float> > &vecModelCoordinates,
                        float x,
                        float y,
                        BoundingBox &bb,
                        const float r = 0.0f,
                        const float s = 1.0f,
                        sf::Color col = sf::Color::White)

{
    // Create translated model vector of coordinate pairs
    vector<pair<float, float> > vecTransformedCoordinates;
    int verts = vecModelCoordinates.size();
    vecTransformedCoordinates.resize(verts);

    // Rotate
    for (int i = 0; i < verts; i++)
    {
        vecTransformedCoordinates[i].first = vecModelCoordinates[i].first * cosf(r) - vecModelCoordinates[i].second * sinf(r);
        vecTransformedCoordinates[i].second = vecModelCoordinates[i].first * sinf(r) + vecModelCoordinates[i].second * cosf(r);
    }

    // Scale
    for (int i = 0; i < verts; i++)
    {
        vecTransformedCoordinates[i].first = vecTransformedCoordinates[i].first * s;
        vecTransformedCoordinates[i].second = vecTransformedCoordinates[i].second * s;
    }

    // Translate
    for (int i = 0; i < verts; i++)
    {
        vecTransformedCoordinates[i].first = vecTransformedCoordinates[i].first + x;
        vecTransformedCoordinates[i].second = vecTransformedCoordinates[i].second + y;
    }

    // create an empty shape
    sf::ConvexShape convex;

    // resize it to verts points
    convex.setPointCount(verts);

    // define the points
     for (int i = 0; i < verts; i++)
    {
        convex.setPoint(i, sf::Vector2f(vecTransformedCoordinates[i % verts].first, vecTransformedCoordinates[i % verts].second));
    }
    convex.setFillColor(col);
    convex.setOutlineThickness(2);

    bb = createBBfromVecModel(vecTransformedCoordinates);

    if(DEBUG_MODE)
    {
        sf::RectangleShape bb_debug;
        bb_debug.setFillColor(sf::Color::Black);
        bb_debug.setSize(sf::Vector2f(bb.width, bb.height));
        bb_debug.setPosition(bb.x, bb.y);
        bb_debug.setOutlineColor(sf::Color::Red);
        bb_debug.setOutlineThickness(2);
        window.draw(bb_debug);
    }

    window.draw(convex);

    bb = createBBfromVecModel(vecTransformedCoordinates);
}

void resetGame(Object &_player, vector<Object> &_bullets, vector<Object> &_asteroids, bool &_dead, int &_score)
{
    _player.x = ScreenWidth / 2.0f;
    _player.y = ScreenHeight / 2.0f;
    _player.dx = 0.0f;
    _player.dy = 0.0f;
    _player.angle = 0.0f;
    _player.osize = 8;

    _bullets.clear();
    _asteroids.clear();

    _asteroids.push_back({ 128, (float)ScreenWidth*0.2f, (float)ScreenHeight*0.2f, 18.0f, -16.0f, 0.0f });
    _asteroids.push_back({ 128, (float)ScreenWidth*0.7f, (float)ScreenHeight*0.7f, -20.0f, 8.0f, 0.0f });

    _dead = false;
    _score = 0;
}

int main()
{
    //TODO: Kysyt‰‰n pelaajan nimi jos saa uuden highscoren

    float padding = 100.0f;
    float box_width = 300.0f;
    float box_height = 100.0f;

    bool show_hs = false;
    bool space_pressed = false;
    bool dead = false;
	bool game_mode = 0;
	bool name_input_mode = 0;

	string name = "";
	string score_s;

    Highscores hs;
    Object player;
    BoundingBox playerBB;

    int score = 0;

    sf::Clock clock;
    sf::Time dtime;
    sf::RenderWindow window(sf::VideoMode(ScreenWidth, ScreenHeight), "Asteroids");
    window.setFramerateLimit(60);

    sf::Vector2i mouse_pos;
    sf::Vector2i mouse_pressed_pos;
    sf::Font font;
    if (!font.loadFromFile("data/fonts/AGENCYB.ttf"))
    {
       cout << "Error loading Fontfile!" << endl;
    }

    sf::SoundBuffer buffer;
    if (!buffer.loadFromFile("data/sounds/pew.wav"))
    {
        cout << "Error loading pew.wav!" << endl;
    }
    sf::Sound sound_pew;
    sound_pew.setBuffer(buffer);

    sf::Text text;

    vector<Object> asteroids;
    vector<Object> bullets;
    vector<Button> menu;

    text.setFont(font);
    score_s = to_string(score);
    text.setString(score_s);
    text.setCharacterSize(48);
    text.setFillColor(sf::Color::Red);

    Button play = {sf::Vector2f(ScreenWidth / 2 - box_width / 2, ScreenHeight / 2 - box_height - padding), "Play", font, 48, sf::Color::Red, sf::Vector2f(box_width, box_height), sf::Color::Blue};
    Button highscore = {sf::Vector2f(ScreenWidth / 2 - box_width / 2, ScreenHeight / 2 - box_height / 2), "Highscore", font, 48, sf::Color::Red, sf::Vector2f(box_width, box_height), sf::Color::Blue};
    Button exit = {sf::Vector2f(ScreenWidth / 2 - box_width / 2, ScreenHeight / 2 + padding), "Exit", font, 48, sf::Color::Red, sf::Vector2f(box_width, box_height), sf::Color::Blue};

    menu.push_back(play);
    menu.push_back(highscore);
    menu.push_back(exit);


    sf::Texture texture;
    if (!texture.loadFromFile("data/img/bullet.png"))
    {
        cout << "Error loading bullet image!" << endl;
    }
    sf::Sprite bullet_img;
    bullet_img.setTexture(texture);
    bullet_img.setOrigin(sf::Vector2f(5.f, 5.f));

    vector<pair<float, float> > vecModelShip;
    vecModelShip.push_back(std::make_pair(0.0f, -4.0f));
    vecModelShip.push_back(std::make_pair(-2.5f, +2.5f));
    vecModelShip.push_back(std::make_pair(0.0f, 0.0f));
    vecModelShip.push_back(std::make_pair(+2.5f, +2.5f));

    playerBB = createBBfromVecModel(vecModelShip);

    vector<pair<float, float>> vecModelAsteroid;

    vector<pair<float, float> > vecBullet;
    vecBullet.push_back(std::make_pair(0.0f, 0.0f));
    vecBullet.push_back(std::make_pair(0.0f, 1.5f));
    vecBullet.push_back(std::make_pair(1.5f, 1.5f));
    vecBullet.push_back(std::make_pair(1.5f, 0.0f));

    int verts = 20;
    for (int i = 0; i < verts; i++)
    {
        float noise = (float)rand() / (float)RAND_MAX * 0.4f + 0.8f;
        vecModelAsteroid.push_back(make_pair(noise * sinf(((float)i / (float)verts) * 6.28318f),
                                             noise * cosf(((float)i / (float)verts) * 6.28318f)));
    }

    resetGame(player, bullets, asteroids, dead, score);

	while (window.isOpen())
    {
        dtime = clock.getElapsedTime();
        sf::Event event;
        window.clear();

        //INPUT
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();

            if (event.type == sf::Event::KeyReleased)
            {
                if (event.key.code == sf::Keyboard::Escape)
                {
                    if(game_mode)
                    {
                        game_mode = 0;
                    }
                    else
                    {
                        window.close();
                    }
                }

                if (event.key.code == sf::Keyboard::Space)
                {
                    if(game_mode)
                    {
                        space_pressed = 0;
                        /*
                        if(bullets.size() < 5)
                        {
                            bullets.push_back({ 1, player.x, player.y, 400.0f * sinf(player.angle) + player.dx, -400.0f * cosf(player.angle) + player.dy, 100.0f, 0 });
                            sound_pew.play();
                        }
                        */
                    }
                }

                if (event.key.code == sf::Keyboard::N)
                {
                    name_input_mode = 1;
                }
            }

            //This is never used. Functioning, just would need to draw stuff on screen for the player.
            if(event.type == sf::Event::TextEntered && name_input_mode == 1)
            {
                if( event.text.unicode < 128 )
                {
                    // return key
                    if( event.text.unicode == 13 )
                    {
                        name_input_mode = 0;
                        //TODO: Save name and score to file

                    }

                    // backspace
                    else if( event.text.unicode == 8 )
                    {
                        if( name.size() > 0 )
                        {
                           name.resize( name.size() - 1 );
                        }
                    }
                    else
                    {
                        name += static_cast<char>(event.text.unicode);
                    }
                }
            cout << name << endl;
            }
        }

        if(dead)
        {
            int index = -1;
            hs.load_highscores();
            for(int i = 0 ; i < 10 ; i++){
                if(score > hs.scores[i].score){
                    index = i;
                    break;
                }
            }

            if(index >= 0){
                for(int i = 8 ; i >= index ; i--){
                    hs.scores[i + 1].name = hs.scores[i].name;
                    hs.scores[i + 1].score = hs.scores[i].score;
                }
                hs.scores[index].name = "Pekka";
                hs.scores[index].score = score;

                hs.save_highscores();
            }

            show_hs = true;
            hs.load_highscores();



            while(show_hs)
            {
                window.clear();
                hs.print_highscores(window, font, index);
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Enter)){show_hs = false;}
                window.display();
            }

            resetGame(player, bullets, asteroids, dead, score);
        }

        if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
        {
            //sf::Vector2f pos = sf::Mouse::getPosition(window);
        }

        //Inputit event loopin ulkopuolella, ei tuu input delayta.

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
        {
            if(game_mode)
                player.angle -= 5.0f * dtime.asSeconds();
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
        {
            if(game_mode)
                player.angle += 5.0f * dtime.asSeconds();
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
        {
            if(game_mode){
                player.dx += sin(player.angle) * 200.0f * dtime.asSeconds();
                player.dy += -cos(player.angle) * 200.0f * dtime.asSeconds();
            }
        }

        if(game_mode){
            //Pelaajan liikuttaminen kiihtyvyden perusteella
            player.x += player.dx * dtime.asSeconds();
            player.y += player.dy * dtime.asSeconds();

            //Alternative shooting

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
            {
                if(!space_pressed)
                {
                    if(bullets.size() < 5)
                    {
                        bullets.push_back({ 1, player.x, player.y, 400.0f * sinf(player.angle) + player.dx, -400.0f * cosf(player.angle) + player.dy, 100.0f, 0 });
                        sound_pew.play();
                    }
                    space_pressed = 1;
                }

            }

            for(unsigned int i = 0 ; i < asteroids.size() ; i++)
            {
                asteroids[i].x += asteroids[i].dx * dtime.asSeconds();
                asteroids[i].y += asteroids[i].dy * dtime.asSeconds();
                asteroids[i].angle += 0.5f * dtime.asSeconds();

                wrapCoordinates(asteroids[i].x, asteroids[i].y, asteroids[i].x, asteroids[i].y);

                drawWireFrameModel(window,
                                   vecModelAsteroid,
                                   asteroids[i].x,
                                   asteroids[i].y,
                                   asteroids[i].angle,
                                   (float)asteroids[i].osize,
                                   sf::Color::Yellow);
            }

            //Screenwrap moduluksella
            player.x = fmod(player.x, ScreenWidth);
            player.y = fmod(player.y, ScreenHeight);
            if(player.x < 0){player.x = ScreenWidth;}
            if(player.y < 0){player.y = ScreenHeight;}
            //wrapCoordinates(player.x, player.y, player.x, player.y);

            vector<Object> newAsteroids;

            // Update Bullets
            for (unsigned int i = 0 ; i < bullets.size() ; i++)
            {
                bullets[i].x += bullets[i].dx * dtime.asSeconds();
                bullets[i].y += bullets[i].dy * dtime.asSeconds();

                wrapCoordinates(bullets[i].x, bullets[i].y, bullets[i].x, bullets[i].y);

                bullets[i].liveTime += dtime.asSeconds();

                bullets[i].angle -= 1.0f * dtime.asSeconds();

                // Check collision with asteroids
                for (unsigned int j = 0 ; j < asteroids.size() ; j++)
                {
                    if(isPointInsideCircle(asteroids[j].x, asteroids[j].y, asteroids[j].osize, bullets[i].x, bullets[i].y))
                    {
                        //remove bullets
                        bullets[i].x = -100;

                        // Create child asteroids in new vector so the current one can iterate
                        if (asteroids[j].osize > 16)
                        {
                            float asteroid_speed = 4 * asteroids[j].dx;
                            float angle1 = ((float)rand() / (float)RAND_MAX) * 6.283185f;
                            float angle2 = ((float)rand() / (float)RAND_MAX) * 6.283185f;
                            newAsteroids.push_back({ (int)asteroids[j].osize >> 1 ,asteroids[j].x, asteroids[j].y, asteroid_speed * sinf(angle1), asteroid_speed * cosf(angle1), 0.0f });
                            newAsteroids.push_back({ (int)asteroids[j].osize >> 1 ,asteroids[j].x, asteroids[j].y, asteroid_speed * sinf(angle2), asteroid_speed * cosf(angle2), 0.0f });
                        }

                        // remove asteroid
                        asteroids[j].x = -100;
                        score += 100;
                    }
                }
            }

            // Append new asteroids to existing vector
            for(unsigned int i = 0 ; i < newAsteroids.size() ; i++)
                asteroids.push_back(newAsteroids[i]);

            if (asteroids.size() > 0)
            {
                auto i = remove_if(asteroids.begin(), asteroids.end(), [&](Object o) { return (o.x < 0); });
                if (i != asteroids.end())
                    asteroids.erase(i);
            }

            if (bullets.size() > 0)
            {
                auto i = remove_if(bullets.begin(), bullets.end(), [&](Object o) { return (o.x < 0); });
                if (i != bullets.end())
                    bullets.erase(i);
            }

            if (asteroids.empty()) // Level complete when no more asteroids
            {
                score += 1000;
                asteroids.clear();
                bullets.clear();

                // Add two new asteroids, but in a place where the player is not, we'll simply
                // add them 90 degrees left and right to the player, their coordinates will
                // be wrapped by the next asteroid update
                asteroids.push_back({ (int)128, 300.0f * sinf(player.angle - 3.14159f/2.0f) + player.x,
                                                  300.0f * cosf(player.angle - 3.14159f/2.0f) + player.y,
                                                  10.0f * sinf(player.angle), 10.0f*cosf(player.angle), 0.0f });

                asteroids.push_back({ (int)128, 300.0f * sinf(player.angle + 3.14159f/2.0f) + player.x,
                                                  300.0f * cosf(player.angle + 3.14159f/2.0f) + player.y,
                                                  10.0f * sinf(-player.angle), 10.0f*cosf(-player.angle), 0.0f });
            }

            if (bullets.size() > 0)
            {
                // Remove bullets that have gone off screen
                //auto i = remove_if(bullets.begin(), bullets.end(), [&](Object o) { return (o.x < 1 || o.y < 1 || o.x >= ScreenWidth - 1 || o.y >= ScreenHeight - 1); });

                auto i = remove_if(bullets.begin(), bullets.end(), [&](Object o) { return (o.liveTime > 5); });
                if (i != bullets.end())
                    bullets.erase(i);
            }

            // Draw Bullets
            for (unsigned int i = 0 ; i < bullets.size() ; i++)
            {
                bullet_img.setPosition(sf::Vector2f(bullets[i].x, bullets[i].y));
                window.draw(bullet_img);
            }

            drawWireFrameModel(window, vecModelShip, player.x , player.y, playerBB, player.angle, player.osize, sf::Color::Blue);

            for (unsigned int i = 0 ; i < asteroids.size() ; i++)
            {
                //if (isPointInsideCircle(asteroids[i].x, asteroids[i].y, asteroids[i].osize, player.x, player.y))
                if(isBBInsideCircle(playerBB, asteroids[i].x, asteroids[i].y, asteroids[i].osize))
                    dead = true;
            }

            if(ScreenWidth - player.x < 60.0f)
            {
                drawWireFrameModel(window, vecModelShip, (player.x - ScreenWidth) , player.y , player.angle, player.osize, sf::Color::Blue);
            }

            if(player.x < 60.0f)
            {
                drawWireFrameModel(window, vecModelShip, (player.x + ScreenWidth) , player.y , player.angle, player.osize, sf::Color::Blue);
            }

            if(ScreenHeight - player.y < 60.0f)
            {
                drawWireFrameModel(window, vecModelShip, player.x, (player.y - ScreenHeight), player.angle, player.osize, sf::Color::Blue);
            }

            if(player.y < 60.0f)
            {
                drawWireFrameModel(window, vecModelShip, player.x, (player.y + ScreenHeight), player.angle, player.osize, sf::Color::Blue);
            }

            score_s = to_string(score);
            text.setString(score_s);
            window.draw(text);
        }
        // !gamemode
        else
        {
            mouse_pos = sf::Mouse::getPosition(window);

            if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
            {
                mouse_pressed_pos = sf::Mouse::getPosition(window);
            }

            for(unsigned int i = 0 ; i < menu.size() ; i++)
            {
                if(mouse_pos.x > menu[i].pos.x && mouse_pos.x < menu[i].pos.x + menu[i].box_size.x &&
                   mouse_pos.y > menu[i].pos.y && mouse_pos.y < menu[i].pos.y + menu[i].box_size.y)
                {
                    menu[i].box_color = sf::Color::Cyan;
                }
                else
                {
                    menu[i].box_color = sf::Color::Blue;
                }
                menu[i].update();
                menu[i].draw(window);

                if(mouse_pressed_pos.x > menu[i].pos.x && mouse_pressed_pos.x < menu[i].pos.x + menu[i].box_size.x &&
                   mouse_pressed_pos.y > menu[i].pos.y && mouse_pressed_pos.y < menu[i].pos.y + menu[i].box_size.y)
                {
                    switch(i){
                    case 0:
                        game_mode = 1;
                        break;
                    case 1:
                        show_hs = true;
                        hs.load_highscores();
                        hs.print_highscores(window, font);
                        window.display();
                        while(show_hs){
                            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space)){show_hs = false;}
                            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Enter)){show_hs = false;}
                        }

                        break;
                    case 2:
                        window.close();
                        break;
                    default:
                        cout << "Main menu error. Case defaulted" << endl;
                    }
                    mouse_pressed_pos.x = 0;
                    mouse_pressed_pos.y = 0;
                }
            }
        }

        clock.restart().asSeconds();

        window.display();
    }
	return 0;
}
