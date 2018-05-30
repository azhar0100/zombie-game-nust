#include <iostream>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <math.h>
#include <vector>
#include "SFMLOrthogonalLayer.hpp"
#include <tmxlite/Map.hpp>
#include <SFML/Audio.hpp>
// enum{
// 	KEY_UP,KEY_DOWN,KEY_RIGHT,KEY_LEFT
// }
// double approximate_to_quadrantal(double angle){
// 	return *std::min_element({90-angle,180-angle,270-angle,360-angle});
// }

/*
Itterheim, Steffen. "Working with tilemaps." Learn iPhone and iPad cocos2d Game Development. Apress, Berkeley, CA, 2010. 211-233.
Game, Competitive Dice. "The Walking Dead™." (2014).
*/

int score=0;

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

class Bullet;

class Collidable{
public:
	static std::vector<Collidable*> all;
	virtual bool is_colliding(const sf::FloatRect &b) = 0;
	virtual void get_hit(const Bullet &b)=0;
	Collidable(){
		all.push_back(this);
	}
};
std::vector<Collidable*> Collidable::all;
sf::Texture shiny_bullet;

class Bullet:public sf::Sprite, public Updatable{



public:
	static std::vector<Bullet*> bullets;
	static std::vector<Bullet*> dead_bullets;

	bool alive = true;
	sf::Vector2f direction;
	Bullet(sf::Vector2f p,sf::Vector2f d){
		setTexture(shiny_bullet);
		setTextureRect(sf::IntRect(63,186,30,10));
		direction = normalize(d,10);
		setOrigin(0,15);
		setPosition((p+sf::Vector2f(0,10)));
		turnto(direction);
		// std::cout << "Bkwas" << std::endl;

	}


	void turnto(sf::Vector2f d){
		direction = d;
		// body->setRotation(std::atan2(d.y,d.x) * (180.0/3.141592653589793238463));
		// feet->setRotation(std::atan2(d.y,d.x) * (180.0/3.141592653589793238463));
		setRotation(std::atan2(d.y,d.x) * (180.0/3.141592653589793238463));

	}

	void moveForward(){
		move(direction);
	}
	void update(){
		if(!alive){
			return;
		}
		moveForward();

		if(getPosition().x < 0 || getPosition().y < 0 || getPosition().x > 4096 || getPosition().y > 4096 ){
			kill();
			return;
		}
		for(int i=0 ; i < Collidable::all.size() ; i++){
			if(Collidable::all[i]->is_colliding(getGlobalBounds())){
				Collidable::all[i]->get_hit(*this);
				kill();
				break;
			}
		}
	}

	static Bullet* spawn(sf::Vector2f p,sf::Vector2f d){
		// for(int i=0 ; i < bullets.size() ; i++){
		// 	if(!bullets[i]->alive){
		// 		bullets[i]->direction = normalize(d,10);
		// 		bullets[i]->setPosition(p);
		// 		bullets[i]->alive = true;

		// 		return bullets[i];
		// 	}
		// }

		Bullet *b;

		if(dead_bullets.size() > 0){
			b = dead_bullets.back();
			dead_bullets.pop_back();
			return b;
		}

		b = new Bullet(p,d);
		bullets.push_back(b);
		return b;
	}

	void kill(){
		setPosition(-1000,-1000);
		direction = sf::Vector2f(0,0);
		alive = false;
		dead_bullets.push_back(this);
		// delete this;
	}

};


std::vector<Bullet*> Bullet::bullets;
std::vector<Bullet*> Bullet::dead_bullets;
sf::Sound sound;

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

class StatefulAnimatedSprite:public AnimatedSprite{
public:
	std::vector<AnimationTextures*> states;
	int current_state = 0;
	StatefulAnimatedSprite(const std::vector<AnimationTextures*> &a):AnimatedSprite(*a[0]){
		states = a;
		if (states.size() > 0 && states[current_state]->textures.size() >0) {
			setTexture((*states[0]->textures[0]));
		}
	}

	void change_state(int i){
		if(! (current_state == i)){
			current_state = i;
			current_texture = 0;
		}
	}
	void virtual update(){
		if(states[current_state]->textures.size() != 0){	
			current_texture = (current_texture+1) % states[current_state]->textures.size();
			setTexture(*(states[current_state]->textures[current_texture]));
		}
	}	
};


enum body_state
{
	idle=0,
	moving=1,
	shooting=2
};

sf::Vector2f player_movement;
std::vector<sf::FloatRect*> collision_tiles;
class Player: public sf::Drawable, public sf::Transformable,public Updatable {
 public:
 
       static AnimationTextures body_textures;
       static AnimationTextures flash_texture;
       static AnimationTextures idle_body_textures;
       static AnimationTextures shooting_body_textures;
       static AnimationTextures feet_textures;
       static AnimationTextures idle_feet_textures;
        
       StatefulAnimatedSprite* feet;
       StatefulAnimatedSprite* body;
       sf::Sprite flash;
       bool show_flash = false;
       body_state fstate = moving;
       body_state bstate = moving;
       
       bool is_shooting = false;
       sf::Vector2f direction;
       float lastShotFiredAt = 0;
       float lastMotionAt = 0;
       float lastupdate = 0;

       int health = 100;



	Player(){
		direction = sf::Vector2f(1,0);
		std::vector<AnimationTextures*> feet_animations;
		feet_animations.push_back(&idle_feet_textures);
		feet_animations.push_back(&feet_textures);

		std::vector<AnimationTextures*> body_animations;
		body_animations.push_back(&idle_body_textures);
		body_animations.push_back(&body_textures);
		body_animations.push_back(&shooting_body_textures);
		body = new StatefulAnimatedSprite(body_animations);
		feet = new StatefulAnimatedSprite(feet_animations);
		flash.setTexture(*flash_texture.textures[0]);
		// flash.setScale(1.0/4,1.0/4);
		setOrigin(95,120);
		flash.setOrigin(23,122);
		// flash.setPosition(getTransform().getInverse().transformPoint(0,0));
		flash.setPosition(292,150);
		flash.setScale(1.0/2,1.0/3);
		setScale(1.0/4,1.0/4);
	}

	void turnto(sf::Vector2f d){
		direction = d;
		// body->setRotation(std::atan2(d.y,d.x) * (180.0/3.141592653589793238463));
		// feet->setRotation(std::atan2(d.y,d.x) * (180.0/3.141592653589793238463));
		setRotation(std::atan2(d.y,d.x) * (180.0/3.141592653589793238463));

	}

    void shoot(){
    	float current_time = globalClock.getElapsedTime().asSeconds(); 
    	// sound.play();
    	if(current_time - lastShotFiredAt > 0.01f){
	    	bstate = shooting;
	    	is_shooting = true;
	    	// Bullet::bullets.push_back(new Bullet(getTransform().transformPoint(sf::Vector2f(292,150)),direction));
	    	Bullet::spawn(getTransform().transformPoint(sf::Vector2f(292,150)),direction);
	    	lastShotFiredAt = current_time;
    	}
    }	

    void update(){
    	std::cout << "Player at "<< getPosition().x << " , " << getPosition().y; 
    	float current_time = globalClock.getElapsedTime().asSeconds();
    	move(player_movement);
        bool obstruct = false;
        for(int i=0;i < collision_tiles.size();i++){
        	// if(is_colliding(*(collision_tiles[i])) ){	
        	if(is_colliding(*(collision_tiles[i])) ){	
	    		obstruct = true;
		    	std::cout << "Player collided with " ;
		    	std::cout << collision_tiles[i]->left << " ";
		    	std::cout << collision_tiles[i]->top << " ";
		    	std::cout << collision_tiles[i]->width << " ";
		    	std::cout << collision_tiles[i]->height << " " << std::endl;
        	} 
        }
        if(obstruct){
	    	move(-player_movement);

	    	obstruct = false;
	    	// bstate = idle;
        }
        std::cout << "player is moving like" << magnitude(player_movement);
    	show_flash = false;
        if(is_shooting){
        	show_flash = false;
        	body->change_state(2);
        	std::cout << "player shoots like" << body->current_state << std::endl;
        	if(body->current_texture == 2){
        		sound.play();
	        	std::cout << "player stops shooting like" << std::endl;
        		is_shooting=false;
        		show_flash = true;
        		// body->change_state(1);
        	}
        }
        if(magnitude(player_movement) == 0){
        	bstate = idle;
        }
        else{
			bstate = moving;
        }
        if (bstate == idle){
        	feet->change_state(idle);
        	if(!is_shooting)
        		body->change_state(idle);
        }
        else{
        	feet->change_state(moving);
        	if(!is_shooting)
       			body->change_state(moving);
        	lastMotionAt = current_time;
        }
        if(current_time - lastupdate > 0.01f){
			feet->update();
			body->update();
			lastupdate = current_time;
        }
	}

	// template<class T>
	bool is_colliding(const sf::FloatRect &sBounds){
		sf::FloatRect bodyBounds = getTransform().transformRect(body->getGlobalBounds());
		// sf::FloatRect sBounds = s.getGlobalBounds();
		// sf::FloatRect &sBounds = s;

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
        if(show_flash){
        	target.draw(flash, states);
        }
    }


};



AnimationTextures Player::flash_texture("Survivor Spine/images/flash",0,0);
AnimationTextures Player::body_textures("Top_Down_Survivor/rifle/move/survivor-move_rifle_",0,19);
AnimationTextures Player::idle_body_textures("Top_Down_Survivor/rifle/idle/survivor-idle_rifle_",0,19);
AnimationTextures Player::shooting_body_textures("Top_Down_Survivor/rifle/shoot/survivor-shoot_rifle_",0,2);
// AnimationTextures Player::body_textures("Top_Down_Survivor/rifle/move/survivor-move_rifle_",0,19);
AnimationTextures Player::feet_textures("Top_Down_Survivor/feet/run/survivor-run_",0,19);
AnimationTextures Player::idle_feet_textures("Top_Down_Survivor/feet/idle/survivor-idle_",0,0);
Player p;

enum zombie_state{
	Attacking, Moving
};

class Zombie:public AnimatedSprite,Updatable,Collidable{

public:
	static AnimationTextures zombie_textures;
	static AnimationTextures attack_textures;
	static AnimationTextures idle_textures;
	static std::vector<Zombie*> zombies;
	sf::Vector2f direction;
	sf::Vector2f target;
	float speed;
	int pos_in_vector;
	bool attacking = false;
	bool random_chosen = false;
	int health = 100;
	float lastupdate = 0;
	zombie_state state = Moving;
	bool alive = true;
	int times_resurrected = 0;

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
	bool is_colliding(const sf::FloatRect &b){
		return b.intersects(getGlobalBounds());
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
	bool colliding(const sf::FloatRect &s){
		// sf::FloatRect bodyBounds = getTransform().transformRect(body->getGlobalBounds());
		sf::FloatRect bodyBounds = getGlobalBounds();
		// sf::FloatRect sBounds = s.getGlobalBounds();
		sf::FloatRect sBounds = s;

		bool cond = bodyBounds.intersects(sBounds);
		if(cond){
			std::cout << "Body at " << bodyBounds.left << " , " << bodyBounds.top << " , " << bodyBounds.width << " , " << bodyBounds.height << std::endl; 
			std::cout << "Rect at " << sBounds.left << " , " << sBounds.top << " , " << sBounds.width << " , " << sBounds.height << std::endl; 
			// std::cout << "rect at " << sBounds.left << " , " << sBounds.top << std::endl; 
		}
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
		if(!alive){
			if(globalClock.getElapsedTime().asSeconds() - lastupdate > 2.f){
				alive = true;
				int place_choice = rand() % 2;
				if(place_choice == 0){
					setPosition(974,877);//NBS
				}
				if(place_choice == 1){
					setPosition(1845,501);//SEECS
				}
			}
			return;
		}(sf::Vector2f({40,5}));
    // sword.setFillColor(sf::Color(255,0,0));


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
		if(magnitude(distance_from_player) <= 50 && ! (state == Attacking)){
			AnimatedSprite::setTextures(attack_textures);
			state = Attacking;
		}
		else if(magnitude(distance_from_player) > 50 && (state == Attacking)){
			AnimatedSprite::setTextures(zombie_textures);
			state = Moving;
		}
		if(state == Attacking){
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

		bool obstruct = false;
        for(int i=0;i < collision_tiles.size();i++){
        	if(colliding(*(collision_tiles[i])) ){	
	    		obstruct = true;
        	} 
        }
        if(obstruct){
	    	move(normalize(movement,-speed));
	    	std::cout << "Zombie collided" << std::endl;
	    	obstruct = false;
	    	// bstate = idle;
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
	void get_hit(const Bullet &b){
		health -= 20;
		std::cout << "Zombie was hit" << std::endl;
		if(health <= 0 ){
			setPosition(-500,-500);
			alive = false;
			times_resurrected++;
			health = 100*times_resurrected ;
			score += health;
			std::cout << "Score:" + std::to_string(score) << std::endl;
		}
	}

	static bool spawn(sf::Vector2f pos){
		for(int i=0;i<zombies.size();i++){
			if(!zombies[i]->alive){
				zombies[i]->alive = true;
				// zombies[i]->health = 100;
				zombies[i]->setPosition(pos);
			}
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
AnimationTextures Zombie::idle_textures("export/skeleton-idle_",0,16);

std::vector<Zombie*> Zombie::zombies;


int main()
{

	sf::SoundBuffer buffer;
	// load something into the sound buffer...
	buffer.loadFromFile("M4A1_Single-Kibblesbob-8540445.wav");

	sound.setBuffer(buffer);
	// sound.play();
	shiny_bullet.loadFromFile("M484BulletCollection1.png");
    std::cout << "Got here";
	bool random_chosen=false;
	srand(time(NULL));
	sf::ContextSettings settings;
	settings.antialiasingLevel = 8;

	sf::RenderWindow window(sf::VideoMode(800, 600), "Zombie", sf::Style::Default, settings);
	sf::View view(sf::FloatRect(0,0,800,600));

    // run the program as long as the window is open
    window.setFramerateLimit(60);
    // window.setVerticalSyncEnabled(true);
	sf::Clock clock;
    sf::Time time;
    sf::Time timeElapsed;
    sf::RectangleShape health(sf::Vector2f(100,10));
    health.setPosition(0,0);
    // sf::Sprite zombie;
    // zombie.setTexture(zombie_textures[0]);
    // zombie.setOrigin(86,129);
    // zombie.setScale(1.0/4,1.0/4);
    p.setPosition(64*32,33*32);


    for(int i=0;i<16;i++){
    	Zombie *z = new Zombie();

    	// z->setPosition(974,677); // For NBS
    	z->setPosition(1845,501); //For seecs
    	// Zombie::zombies.push_back(z);
    }
    // sf::RectangleShape sword(sf::Vector2f({40,5}));
    // sword.setFillColor(sf::Color(255,0,0));

    int k=0;
	tmx::Map map;
    // map.load("tiled/zombiegame org.tmx");
    // map.load("tiled/zombiegame1.tmx");
    map.load("tiled/zorg_2.tmx");

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
			if(tileIDs[ i*100 + j].ID == 9327 ){
				sf::FloatRect *tile = new sf::FloatRect(sf::Vector2f(32*j,32*i),sf::Vector2f(32,32));
				// tile->setPosition(32*j,32*i);
				// std::cout << "collision at " << tile->getPosition().x << " , " << tile->getPosition().y << std::endl;
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
        std::cout << "FPS at " <<1.0f/time.asSeconds() << std::endl;

        sf::Vector2f movement(0,0);
        player_movement = movement;
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

        player_movement = normalize(movement, movementmagnitude);


		mousePos = sf::Mouse::getPosition(window) ;
        p.turnto( sf::Vector2f(mousePos.x,mousePos.y) - sf::Vector2f(400,300));

		for(int i = 0;i < Updatable::updatables.size() ; i++){
			if(Updatable::updatables[i] != NULL){
				Updatable::updatables[i]->update();
			}
		}

		view = sf::View(p.getPosition(),sf::Vector2f(800,600));
		// view.zoom(0.8);
		window.setView(view);

	    window.clear(sf::Color(100,255,100));

	    for( int i=0;i<n;i++){
		    // std::cout << "Got here too";
		    if(is_background_layer(i)){
	    		window.draw(*(layers[i]));
		    }
	    }
	    if(Bullet::bullets.size() > 1000){
	    	std::cout << "Clearing time;"<< std::endl;
	    	Bullet::bullets.clear();
	    	// Updatable::updatables.erase( std::remove(Updatable::updatables.begin(), Updatable::updatables.end(), NULL), Updatable::updatables.end() )
	    }
		for(int i=0;i<Bullet::bullets.size();i++){
			window.draw(*(Bullet::bullets[i]));
		}
	    window.draw(p);
	    for(int i = 0 ; i<Zombie::zombies.size();i++){
	    	window.draw(*Zombie::zombies[i]);
		}
		// window.draw(z);
	    // if(draw_sword)
	    // 	window.draw(sword);

	    for( int i=0;i<n-1;i++){
	    // for( int i=0;i<n;i++){
		    // std::cout << "Got here too";
		    if(!is_background_layer(i)){
	    		window.draw(*(layers[i]));
		    }
	    }

	    // for( int i=0;i<collision_tiles.size();i++){
		   //  // std::cout << "Got here too";
		   //  window.draw(*collision_tiles[i]);
	    // }

	    health.setSize(sf::Vector2f((p.health / 100.0) * 800,10.0));
	    std::cout << "health is at : "<< p.health << std::endl;
	    health.setPosition(window.mapPixelToCoords(sf::Vector2i(0,0)));
	    window.draw(health);

	    if (p.health <= 0){
	    	return 0;
	    }

	    clock.restart();
	    window.display();
		std::cout << "size of updatables" << Updatable::updatables.size();
    }

    // window.draw();

    return 0;
}
