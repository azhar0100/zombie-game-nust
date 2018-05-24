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

bool is_parallel(const sf::Vector2f& a,const sf::Vector2f& b){
	return a.x/b.x == a.y/b.y;
}

double magnitude(const sf::Vector2f& source){
	return sqrt((source.x * source.x) + (source.y * source.y));	
}

sf::Vector2f normalize(const sf::Vector2f& source,double magnitude=1)
{
    float length = sqrt((source.x * source.x) + (source.y * source.y));
    if (length != 0)
        return sf::Vector2f(source.x * magnitude / length, source.y * magnitude / length);
    else
        return source;
}
class Bullet:public sf::CircleShape{


public:
	sf::Vector2f direction;
	Bullet(sf::Vector2f p,sf::Vector2f d):sf::CircleShape(2){
		direction = normalize(d,10);
		setPosition(p);
		// std::cout << "Bkwas" << std::endl;
	}

	void moveForward(){
		move(direction);
	}

};

std::vector<Bullet*> bullets;

class AnimatedSprite: public sf::Sprite{
public:

	std::vector<sf::Texture*> textures;
	std::size_t current_texture;

	AnimatedSprite(std::vector<sf::Texture*> t): textures(t){
		setTexture(textures[0]);
	}

	void update(){
		k = (k+1) % textures.size();
		setTexture(zombie_textures[k]);
	}
}

class Player: public sf::Sprite {
public:

	static std::array<sf::Texture,20> body_textures;
	static std::array<sf::Texture,20> feet_textures;
	
	sf::Sprite feet;

	sf::Vector2f direction;
	int k = 0;


	Player(){
		feet.setTexture(feet_textures[0]);
		setTexture(body_textures[0]);
		setOrigin(95,120);
		setScale(1.0/4,1.0/4);
		feet.setOrigin(95,120);
		feet.setScale(1.0/4,1.0/4);
	}

	void turnto(sf::Vector2f d){
		direction = d;
		setRotation(std::atan2(d.y,d.x) * (180.0/3.141592653589793238463));
		feet.setRotation(std::atan2(d.y,d.x) * (180.0/3.141592653589793238463));
	}

    void shoot(){
    	bullets.push_back(new Bullet(getTransform().transformPoint(sf::Vector2f(292,150)),direction));
    }

	void update(){
		k = (k+1) % 20;
		setTexture(body_textures[k]);
		feet.setTexture(feet_textures[k]);
	}

// private:
//     virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const
//     {
//         // apply the entity's transform -- combine it with the one that was passed by the caller
//         states.transform *= getTransform(); // getTransform() is defined by sf::Transformable

//         // apply the texture
//         // states.texture = &m_texture;

//         // you may also override states.shader or states.blendMode if you want

//         // draw the vertex array
//         // target.draw(this,states);
//         target.draw(feet, states);
//     }


};

class Zombie:public sf::Sprite{
public:
	static std::array<sf::Texture,17> zombie_textures;
	sf::Vector2f direction;
	int k = 0;
	bool random_chosen = false;
	int health = 100;

	Zombie(){
		direction = sf::Vector2f(10.f,0);
		setOrigin(86,129);
		setScale(1.0/4,1.0/4);
	}

	void turnto(sf::Vector2f d){
		direction = d;
		setRotation( std::atan2(d.y,d.x) * (180.0/3.141592653589793238463) );
	}

	void update(){
		k = (k+1) % 17;
		setTexture(zombie_textures[k]);
		if(health < 0 ){
			setPosition (0,0);
			health = 100;
		}
	}
};

bool is_background_layer(int n){
	int background_layers[7] = {0,1,4,14,15,16,17};
	for(int i=0;i<7;i++){
		if(n == i){
			return true;
		}
	}
	return false;
}
std::array<sf::Texture,17> Zombie::zombie_textures;
std::array<sf::Texture,20> Player::body_textures;
std::array<sf::Texture,20> Player::feet_textures;

std::vector<Zombie*> zombies;


int main()
{
	bool random_chosen=false;
	srand(time(NULL));
	sf::ContextSettings settings;
	settings.antialiasingLevel = 8;

	sf::RenderWindow window(sf::VideoMode(800, 600), "Zombie", sf::Style::Default, settings);
	sf::View view(sf::FloatRect(0,0,800,600));

    // run the program as long as the window is open
    window.setFramerateLimit(60);
    sf::Clock globalClock;
	sf::Clock clock;
    sf::Time time;
    sf::Time timeElapsed;

    // sf::Vector2f movement;
    // sf::CircleShape player();
    // bool draw_sword = false;

    // "Top_Down_Survivor/feet/run/survivor-run_0.png";

    for(int i=0;i<=20;i++){
    // "Top_Down_Survivor/rifle/move/survivor-move_rifle_0.png";
    	if(!Player::body_textures[i].loadFromFile("Top_Down_Survivor/rifle/move/survivor-move_rifle_" + std::to_string(i) + ".png")){
			std::cout << "Failed at " << i << std::endl;
		}
	}

    for(int i=0;i<=20;i++){
    // "Top_Down_Survivor/rifle/move/survivor-move_rifle_0.png";
    	if(!Player::feet_textures[i].loadFromFile("Top_Down_Survivor/feet/run/survivor-run_" + std::to_string(i) + ".png")){
			std::cout << "Failed at " << i << std::endl;
		}
	}

    for(int i=0;i<=16;i++){
		std::cout << "export/skeleton-move_" + std::to_string(i) + ".png"<<std::endl;
		if(!Zombie::zombie_textures[i].loadFromFile("export/skeleton-move_" + std::to_string(i) + ".png")){
			std::cout << "Failed at " << i << std::endl;
		}
    }

    Player p;
    // sf::Sprite zombie;
    // zombie.setTexture(zombie_textures[0]);
    // zombie.setOrigin(86,129);
    // zombie.setScale(1.0/4,1.0/4);

    std::vector<Zombie*> zombies;

    for(int i=0;i<16;i++){
    	Zombie *z = new Zombie();
    	z->setPosition(rand() % 512,rand() % 512);
    	zombies.push_back(z);
    }
    // sf::RectangleShape sword(sf::Vector2f({40,5}));
    // sword.setFillColor(sf::Color(255,0,0));

    int k=0;
	tmx::Map map;
    map.load("tiled/zombiegame1.tmx");

    std::size_t n = map.getLayers().size();
    std::vector<MapLayer*> layers;
    for(int i=0;i<n;i++){
    	// MapLayer l(map,i);
    	layers.push_back(new MapLayer(map,i));
    }

    while (window.isOpen())
    {	
        // check all the window's events that were triggered since the last iteration of the loop
        sf::Event event;
        time = clock.getElapsedTime();
        timeElapsed = globalClock.getElapsedTime();
        // std::cout << 1.0f/time.asSeconds() << std::endl;

        sf::Vector2f movement;
        sf::Vector2f input;
        sf::Vector2f direction;
        while (window.pollEvent(event))
        {

            // "close requested" event: we close the window
            if (event.type == sf::Event::Closed)
                window.close();

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
			{
				input = sf::Vector2f(5,0);
			    // left key is pressed: move our character
			    if (!is_parallel(movement,input)){
			    	movement += input;
			    }
			    // p.turnto(sf::Vector2f(5,0));
			}
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
			{
			    // left key is pressed: move our character
			    input = sf::Vector2f(-5, 0);
			    if (!is_parallel(movement,input)){
			    	movement += input;
			    }
			    // p.turnto(sf::Vector2f(-5,0));
			}
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
			{
			    // left key is pressed: move our character
			    input = sf::Vector2f(0, 5);
			    if (!is_parallel(movement,input)){
			    	movement += input;
			    }
				// p.turnto(sf::Vector2f(0,5));
			}
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
			{
			    // left key is pressed: move our character
			    input = sf::Vector2f(0, -5);
				if (!is_parallel(movement,input)){
			    	movement += input;
			    }
			    // p.turnto(sf::Vector2f(0,-5));
			}
			

 				
		 //        if(btn_quit.IsIn(event.MouseMoveEvent.x, event.MouseMoveEvent.x){
		 //            btn_quit.RenderImg(window,"button_on.png");
		 //        } else {
		 //            btn_quit.RenderImg(window,"button.png");
		 //        }
			// }
   //      	}	


			if (event.type == sf::Event::TextEntered)
			{
				if (event.text.unicode ==  'x')
					p.shoot();
					std::cout << "ASCII character typed: " << static_cast<char>(event.text.unicode) << std::endl;
			}

	    }
        p.move(normalize(movement,2000*time.asSeconds()));
        sf::Vector2i mousePos = sf::Mouse::getPosition(window) ;
        p.turnto( sf::Vector2f(mousePos.x,mousePos.y) - sf::Vector2f(400,300));
  //       sf::Vector2f z_pos = z.getPosition();
  //       sf::FloatRect prect(z_pos.x-20,z_pos.y-20,z_pos.x+40,z_pos.y+20);
        for(int i=0;i<bullets.size();i++){
			bullets[i]->moveForward();
			for(int j=0;j<zombies.size();j++){
				if((*bullets[i]).getGlobalBounds().intersects(zombies[j]->getGlobalBounds())){
					zombies[j]->health -= 25;
					bullets[i]->direction = sf::Vector2f(0,0);
					bullets[i]->setPosition( sf::Vector2f(-100,-100) );
					std::cout << "Zombie was hit" << std::endl;
				}
			}
		}

		for(int i = 0 ; i<zombies.size();i++){			
			Zombie *z = zombies[i];

			sf::Vector2f del = p.getPosition() - z->getPosition();
			if( magnitude(del) <= 500 ){
				z->move(normalize(del));
				z->turnto(del);
				z->random_chosen = false;
			}
			else if(z->random_chosen){
				z->move(normalize(z->direction));
			}
			else{				
				z->turnto(sf::Vector2f(rand() % 4 - 2,rand() % 4 - 2));
				z->random_chosen = true;
			}
		}

		if( timeElapsed.asSeconds() > 0.2f ){
			for(int i = 0 ; i<zombies.size();i++)
				zombies[i]->update();
			p.update();
			globalClock.restart();
		}

		view = sf::View(p.getPosition(),sf::Vector2f(800,600));
		// view.zoom(0.8);
		window.setView(view);

	    window.clear(sf::Color());

	    for( int i=0;i<n;i++){
		    // std::cout << "Got here too";
		    if(is_background_layer(i)){
	    		window.draw(*(layers[i]));
		    }
	    }
		for(int i=0;i<bullets.size();i++){
			window.draw(*(bullets[i]));
		}
	    window.draw(p);
	    for(int i = 0 ; i<zombies.size();i++){
	    	window.draw(*zombies[i]);
		}
		// window.draw(z);
	    // if(draw_sword)
	    // 	window.draw(sword);

	    for( int i=0;i<n;i++){
		    // std::cout << "Got here too";
		    if(!is_background_layer(i)){
	    		window.draw(*(layers[i]));
		    }
	    }

	    clock.restart();
	    window.display();
    }


    // window.draw();

    return 0;
}

