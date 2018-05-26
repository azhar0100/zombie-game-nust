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



sf::Clock globalClock;

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

// bool is_inside_rectangle(const sf::Vector2f& r,const sf::RectangleShape& s){
// 	sf::Vector2f i = s.getPosition();
// 	sf::Vector2f f = i + s.getSize();
// 	return r.x > i.x && sma
// }

class Updatable{
public:
	static std::vector<Updatable*> updatables;
	virtual void update() = 0;

	Updatable(){
		updatables.push_back(this);
	}

};

std::vector<Updatable*> Updatable::updatables;

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

class AnimationTextures{
public:
	std::vector<sf::Texture*> textures;

	AnimationTextures(std::string texture_path_prefix ,int begin, int end){
	    for(int i=begin;i<=end;i++){
	    	sf::Texture* t = new sf::Texture();
	    	textures.push_back(t);
	    	if(! (textures.back()->loadFromFile(texture_path_prefix + std::to_string(i) + ".png")) ){
				std::cout << "Failed at " << i << std::endl;
			}
		}
	}			
};

class AnimatedSprite: public sf::Sprite{
public:

	std::vector<sf::Texture*> textures;
	std::size_t current_texture = 0;

	AnimatedSprite(std::vector<sf::Texture*> t): textures(t){
		if (textures.size() > 0) {
			setTexture((*textures[0]));
		}
	}
	AnimatedSprite(const AnimationTextures &t): textures(t.textures){
		if (textures.size() > 0) {
			setTexture((*textures[0]));
		}
	}

	void setTextures(std::vector<sf::Texture*> t){
		textures = t;
		if (textures.size() > 0) {
			setTexture((*textures[0]));
		}
		current_texture = 0;
	}

	void setTextures(const AnimationTextures &t){
		setTextures(t.textures);
	}

	void virtual update(){
		if(textures.size() != 0){	
			current_texture = (current_texture+1) % textures.size();
			setTexture(*(textures[current_texture]));
		}
	}
};

class Player: public sf::Drawable, public sf::Transformable,public Updatable {
 public:
 
       static AnimationTextures body_textures;
       static AnimationTextures feet_textures;
        
       AnimatedSprite* body;
       AnimatedSprite* feet;
       sf::Vector2f direction;

       int health = 100;



	Player(){
		direction = sf::Vector2f(1,0);
		body = new AnimatedSprite(body_textures.textures);
		feet = new AnimatedSprite(feet_textures.textures);
		setOrigin(95,120);
		setScale(1.0/4,1.0/4);
	}

	void turnto(sf::Vector2f d){
		direction = d;
		// body->setRotation(std::atan2(d.y,d.x) * (180.0/3.141592653589793238463));
		// feet->setRotation(std::atan2(d.y,d.x) * (180.0/3.141592653589793238463));
		setRotation(std::atan2(d.y,d.x) * (180.0/3.141592653589793238463));

	}

    void shoot(){
    	bullets.push_back(new Bullet(getTransform().transformPoint(sf::Vector2f(292,150)),direction));
    }	

    void update(){
		feet->update();
		body->update();
	}

	// template<class T>
	bool is_colliding(const sf::RectangleShape &s){
		sf::FloatRect bodyBounds = getTransform().transformRect(body->getGlobalBounds());
		sf::FloatRect sBounds = s.getGlobalBounds();

		bool cond = bodyBounds.intersects(sBounds);
		if(cond){
			std::cout << "Body at " << bodyBounds.left << " , " << bodyBounds.top << " , " << bodyBounds.width << " , " << bodyBounds.height << std::endl; 
			std::cout << "Rect at " << sBounds.left << " , " << sBounds.top << " , " << sBounds.width << " , " << sBounds.height << std::endl; 
			// std::cout << "rect at " << sBounds.left << " , " << sBounds.top << std::endl; 
		}
		return cond;
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
        // target.draw(this,states);
        target.draw(*feet, states);
        target.draw(*body, states);
    }


};

AnimationTextures Player::body_textures("Top_Down_Survivor/rifle/move/survivor-move_rifle_",0,19);
AnimationTextures Player::feet_textures("Top_Down_Survivor/feet/run/survivor-run_",0,19);
Player p;

class Zombie:public AnimatedSprite,Updatable{

public:
	static AnimationTextures zombie_textures;
	static AnimationTextures attack_textures;
	static std::vector<Zombie*> zombies;
	sf::Vector2f direction;
	sf::Vector2f target;
	float speed;
	int pos_in_vector;
	bool attacking = false;
	bool random_chosen = false;
	int health = 100;
	float lastupdate = 0;

	Zombie():AnimatedSprite(zombie_textures){
		target = sf::Vector2f( (rand() % 5 ) * 10 , (rand() % 5 ) * 10 );
		speed = 1 + 0.1f * ( rand() % 5 - 2);
		direction = sf::Vector2f(10.f,0);
		setOrigin(86,129);
		setScale(1.0/4,1.0/4);
		zombies.push_back(this);
		pos_in_vector = zombies.size() - 1;
	}

	void turnto(sf::Vector2f d){
		direction = d;
		setRotation( std::atan2(d.y,d.x) * (180.0/3.141592653589793238463) );
	}

	bool is_colliding(const Zombie &s){
		// sf::FloatRect bodyBounds = getTransform().transformRect(body->getGlobalBounds());
		// sf::Transform t;
		// t.scale(sf::Vector2f(0.1,0.1),getOrigin());
		// t.combine(getTransform());
		// if(cond){
		const auto &z1 = getGlobalBounds();
		const auto &z2 = s.getGlobalBounds();
		
		bool cond = z1.intersects(z2);
		if(cond){
			std::cout << "Z1 at " << z1.left << " , " << z1.top << " , " << z1.width << " , " << z1.height << std::endl; 
			std::cout << "Z2 at " << z2.left << " , " << z2.top << " , " << z2.width << " , " << z2.height << std::endl; 
		}
		// 	std::cout << "Rect at " << sBounds.left << " , " << sBounds.top << " , " << sBounds.width << " , " << sBounds.height << std::endl; 
		// 	// std::cout << "rect at " << sBounds.left << " , " << sBounds.top << std::endl; 
		// }
		// cond = false;
		return cond;
	}

	// bool is_colliding_with_any_other_zombie(){
	// 	for(int i = 0; i < zombies.size(); i++ ){
	// 		sf::Vector2f distance_from_this_other_zombie = getPosition() - zombies[i]->getPosition(); 
	// 		if( is_colliding(*zombies[i]) && zombies[i] != this ){
	// 			return;
	// 		}
	// 	}

	// }


	void update(){
		sf::Vector2f distance_from_player = p.getPosition() + target - getPosition();
		sf::Vector2f del(0,0); 
		del = normalize(distance_from_player);
		bool too_close = false;
		sf::Vector2f movingAwayCoeff(0,0);
		for(int i = 0; i < zombies.size(); i++ ){
			sf::Vector2f distance_from_this_other_zombie = getPosition() - zombies[i]->getPosition(); 
			if( magnitude(distance_from_this_other_zombie) < 100 && i != pos_in_vector ){
				movingAwayCoeff +=  normalize(distance_from_this_other_zombie,2);
			}
			// if( magnitude(distance_from_this_other_zombie) < 500 && zombies[i] != this ){

			// }
		}
		movingAwayCoeff = normalize(movingAwayCoeff);

		// del += normalize(movingAwayCoeff);
		std::cout << "movingAwayCoeff :" << movingAwayCoeff.x << "  " << movingAwayCoeff.y << std::endl;
		// if(too_close){

		// }

		sf::Vector2f movement(0,0);
		if(magnitude(distance_from_player) <= 50 && !attacking){
			AnimatedSprite::setTextures(attack_textures);
			attacking = true;
		}
		else if(magnitude(distance_from_player) > 50 && attacking){
			AnimatedSprite::setTextures(zombie_textures);
			attacking = false;
		}
		if(attacking){
			if(AnimatedSprite::current_texture == 8){
				p.health -= 1;
			}
		}
		else{	

			if( magnitude(distance_from_player) <= 500 ){
				// movement = normalize(normalize(distance_from_player) + normalize(del) + normalize(movingAwayCoeff));
				movement = normalize(del + normalize(movingAwayCoeff));
				random_chosen = false;
			}
			else if(random_chosen){
				movement = normalize(normalize(direction) + normalize(movingAwayCoeff));
			}
			else{				
				turnto(sf::Vector2f(rand() % 4 - 2,rand() % 4 - 2));
				random_chosen = true;
			}
			turnto(del);
			move(normalize(movement,speed));
		}

		// for(int i = 0; i < zombies.size(); i++ ){
		// 	if( is_colliding(*zombies[i]) && i != pos_in_vector ){
		// 		move(normalize(movement,-1));				
		// 	}
		// }

		if(globalClock.getElapsedTime().asSeconds() - lastupdate > 0.1f){
			AnimatedSprite::update();
			lastupdate = globalClock.getElapsedTime().asSeconds();
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
AnimationTextures Zombie::zombie_textures("export/skeleton-move_",0,16);
AnimationTextures Zombie::attack_textures("export/skeleton-attack_",0,8);

std::vector<Zombie*> Zombie::zombies;


int main()
{
    std::cout << "Got here";
	bool random_chosen=false;
	srand(time(NULL));
	sf::ContextSettings settings;
	settings.antialiasingLevel = 8;

	sf::RenderWindow window(sf::VideoMode(800, 600), "Zombie", sf::Style::Default, settings);
	sf::View view(sf::FloatRect(0,0,800,600));

    // run the program as long as the window is open
    window.setFramerateLimit(60);
    window.setVerticalSyncEnabled(true);
	sf::Clock clock;
    sf::Time time;
    sf::Time timeElapsed;
    std::vector<sf::RectangleShape*> collision_tiles;
    sf::RectangleShape health(sf::Vector2f(100,10));
    health.setPosition(0,0);
    // sf::Sprite zombie;
    // zombie.setTexture(zombie_textures[0]);
    // zombie.setOrigin(86,129);
    // zombie.setScale(1.0/4,1.0/4);


    for(int i=0;i<16;i++){
    	Zombie *z = new Zombie();
    	z->setPosition(rand() % 1024,rand() % 1024);
    	// Zombie::zombies.push_back(z);
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

	auto& mlayers = map.getLayers();
	const auto& layer = *dynamic_cast<const tmx::TileLayer*>(mlayers[n-1].get());
	const auto& tileIDs = layer.getTiles();
	// cout << map.getTileCount().y << endl;
	for(int i=0;i<=100;i++){
		for(int j=0;j<=100;j++){
			// std::cout << "collision at " << tile->getPosition().x << " , " << tile->getPosition().y;
			if(tileIDs[ i*100 + j].ID == 5951 ){
				sf::RectangleShape *tile = new sf::RectangleShape(sf::Vector2f(32,32));
				tile->setPosition(32*j,32*i);
				std::cout << "collision at " << tile->getPosition().x << " , " << tile->getPosition().y << std::endl;
				collision_tiles.push_back(tile);
			}			
		}
	}

    while (window.isOpen())
    {	
        // check all the window's events that were triggered since the last iteration of the loop
        sf::Event event;
        time = clock.getElapsedTime();
        timeElapsed = globalClock.getElapsedTime();
        std::cout << 1.0f/time.asSeconds() << std::endl;

        sf::Vector2f movement;
        sf::Vector2f input;
        sf::Vector2f direction;
        sf::Vector2i mousePos;
        while (window.pollEvent(event))
        {

            // "close requested" event: we close the window
            if (event.type == sf::Event::Closed)
                window.close();
	    }
		if(sf::Keyboard::isKeyPressed(sf::Keyboard::X))
		{
				p.shoot();
				std::cout << "ASCII character typed: " << static_cast<char>(event.text.unicode) << std::endl;
		}
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
		

	    // p_new_pos = p.getPosition() + normalize(movement,1000*time.asSeconds());
        // sf::Vector2f motion = normalize(movement,1000*time.asSeconds());

		float movementmagnitude = 500*time.asSeconds();
        p.move( normalize(movement, movementmagnitude) );
        bool obstruct = false;
        for(int i=0;i < collision_tiles.size();i++){
        	if(p.is_colliding(*(collision_tiles[i])) ) 
	    		obstruct = true;
        }
        if(obstruct){
	    	p.move(normalize(movement,-movementmagnitude));
	    	std::cout << "Player collided" << std::endl;
	    	obstruct = false;
        }

		mousePos = sf::Mouse::getPosition(window) ;
        p.turnto( sf::Vector2f(mousePos.x,mousePos.y) - sf::Vector2f(400,300));
  //       sf::Vector2f z_pos = z.getPosition();
  //       sf::FloatRect prect(z_pos.x-20,z_pos.y-20,z_pos.x+40,z_pos.y+20);
        for(int i=0;i<bullets.size();i++){
			bullets[i]->moveForward();
			for(int j=0;j<Zombie::zombies.size();j++){
				if((*bullets[i]).getGlobalBounds().intersects(Zombie::zombies[j]->getGlobalBounds())){
					Zombie::zombies[j]->health -= 25;
					bullets[i]->direction = sf::Vector2f(0,0);
					bullets[i]->setPosition( sf::Vector2f(-100,-100) );
					std::cout << "Zombie was hit" << std::endl;
				}
			}
		}

		// for(int i = 0 ; i<Zombie::zombies.size();i++){			
		// 	Zombie *z = Zombie::zombies[i];
		// 	z->update(p.getPosition() - z->getPosition(),timeElapsed.asSeconds());
		// }

		// // if( timeElapsed.asSeconds() > 0.05f ){
		// 	// for(int i = 0 ; i<Zombie::zombies.size();i++)
		// 	// 	Zombie::zombies[i]->update();
		// 	p.update();
		// 	globalClock.restart();
		// // }

		for(int i = 0;i < Updatable::updatables.size() ; i++){
			Updatable::updatables[i]->update();
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
	    for(int i = 0 ; i<Zombie::zombies.size();i++){
	    	window.draw(*Zombie::zombies[i]);
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

	    for( int i=0;i<collision_tiles.size();i++){
		    // std::cout << "Got here too";
		    window.draw(*collision_tiles[i]);
	    }

	    health.setSize(sf::Vector2f((p.health / 100.0) * 200,10.0));
	    std::cout << "health is at : "<< p.health << std::endl;
	    health.setPosition(window.mapPixelToCoords(sf::Vector2i(0,0)));
	    window.draw(health);

	    clock.restart();
	    window.display();
		std::cout << "size of updatables" << Updatable::updatables.size();
    }

    // window.draw();

    return 0;
}

