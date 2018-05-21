#include <iostream>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <math.h>
#include <vector>
#include "SFMLOrthogonalLayer.hpp"
#include <tmxlite/Map.hpp>

// enum{
// 	KEY_UP,KEY_DOWN,KEY_RIGHT,KEY_LEFT
// }
// double approximate_to_quadrantal(double angle){
// 	return *std::min_element({90-angle,180-angle,270-angle,360-angle});
// }
bool is_perpendicular(const sf::Vector2f& a,const sf::Vector2f& b){
	return a.x*b.x + a.y*b.y == 0;
}

sf::Vector2f normalize(const sf::Vector2f& source,double magnitude=1)
{
    float length = sqrt((source.x * source.x) + (source.y * source.y));
    if (length != 0)
        return sf::Vector2f(source.x * magnitude / length, source.y * magnitude / length);
    else
        return source;
}

class Player: public sf::Drawable, public sf::Transformable{
public:
	sf::CircleShape body;
	// sf::Triangle
	sf::ConvexShape face;
	sf::Vector2f direction;

	Player(){

		// resize it to 5 points
		face.setPointCount(3);

		// define the points
		face.setPoint(0, sf::Vector2f(0, 0));
		face.setPoint(1, sf::Vector2f(20, 20));
		face.setPoint(2, sf::Vector2f(0, 40));

		body = sf::CircleShape(20);
		face.setPosition(getPosition() + sf::Vector2f({40,0}));
		setOrigin(20,20);
	}

	void turnto(sf::Vector2f d){
		direction = d;
		setRotation(std::atan2(d.y,d.x) * (180.0/3.141592653589793238463));
		// this->setRotation(90);
	}


private:

    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const
    {
        // apply the entity's transform -- combine it with the one that was passed by the caller
        states.transform *= getTransform(); // getTransform() is defined by sf::Transformable

        // apply the texture
        // states.texture = &m_texture;

        // you may also override states.shader or states.blendMode if you want

        // draw the vertex array
        target.draw(body, states);
        target.draw(face, states);
    }




};

int main()
{
	sf::ContextSettings settings;
	settings.antialiasingLevel = 8;

	sf::RenderWindow window(sf::VideoMode(800, 600), "SFML shapes", sf::Style::Default, settings);
	sf::View view(sf::FloatRect(0,0,800,600));

	bool keys[4] = {0,1,2,3};
    // run the program as long as the window is open
    window.setFramerateLimit(60);
	sf::Clock clock;
    sf::Time time;

    // sf::Vector2f movement;
    // sf::CircleShape player();
    // bool draw_sword = false;
    Player p;
    Player z;
    // sf::RectangleShape sword(sf::Vector2f({40,5}));
    // sword.setFillColor(sf::Color(255,0,0));

	tmx::Map map;
    map.load("tiled/zombiegame.tmx");

    std::size_t n = map.getLayers().size();
    std::vector<MapLayer*> layers;
    for(int i=0;i<n;i++){
    	// MapLayer l(map,i);
    	layers.push_back(new MapLayer(map,i));
    }

 //    MapLayer layerZero(map, 0);
	// MapLayer layerOne(map, 1);
	// MapLayer layerTwo(map, 2);
	// MapLayer layer3(map, 3);
	// MapLayer layer4(map, 4);
	// MapLayer layer5(map, 5);

    while (window.isOpen())
    {
        // check all the window's events that were triggered since the last iteration of the loop
        sf::Event event;
        time = clock.getElapsedTime();
        std::cout << 1.0f/time.asSeconds() << std::endl;

        sf::Vector2f movement;
        sf::Vector2f input;
        while (window.pollEvent(event))
        {

            // "close requested" event: we close the window
            if (event.type == sf::Event::Closed)
                window.close();

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
			{
				input = sf::Vector2f({5,0});
			    // left key is pressed: move our character
			    if (is_perpendicular(movement,input)){
			    	movement += input;
			    }
			    p.turnto(sf::Vector2f({5,0}));
			}
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
			{
			    // left key is pressed: move our character
			    input = sf::Vector2f(-5, 0);
			    if (is_perpendicular(movement,input)){
			    	movement += input;
			    }
			    p.turnto(sf::Vector2f({-5,0}));
			}
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
			{
			    // left key is pressed: move our character
			    input = sf::Vector2f(0, 5);
			    if (is_perpendicular(movement,input)){
			    	movement += input;
			    }
				p.turnto(sf::Vector2f({0,5}));
			}
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
			{
			    // left key is pressed: move our character
			    input = sf::Vector2f(0, -5);
				if (is_perpendicular(movement,input)){
			    	movement += input;
			    }
			    p.turnto(sf::Vector2f({0,-5}));
			}

			// if (sf::Keyboard::isKeyPressed(sf::Keyboard::X))
			// {
			//     // left key is pressed: move our character

			//     sf::Vector2f pos = player.getPosition();
			//     pos += {40,10};
			//     sword.setPosition(pos);
			//     draw_sword = true;

			// }else{
			// 	draw_sword = false;
			// }


        }
        p.move(normalize(movement,600*time.asSeconds()));

			sf::Vector2f del = p.getPosition() - z.getPosition();
			z.move(normalize(del));
			z.turnto(del);

		view = sf::View(p.getPosition(),sf::Vector2f(800,600));
		view.zoom(1);
		window.setView(view);

	    window.clear(sf::Color());

	    for( int i=0;i<n;i++){
	    	window.draw(*(layers[i]));
	    }

  //       window.draw(layerZero);
  //       window.draw(layerOne);
		// window.draw(layerTwo);
		// window.draw(layer3);
		// window.draw(layer4);
		// window.draw(layer5);
	    window.draw(p);
	    window.draw(z);
	    // if(draw_sword)
	    // 	window.draw(sword);

	    clock.restart();
	    window.display();
    }


    // window.draw();

    return 0;
}

