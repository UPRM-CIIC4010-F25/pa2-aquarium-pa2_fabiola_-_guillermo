#include "Aquarium.h"
#include <cstdlib>
#include <cmath>


string AquariumCreatureTypeToString(AquariumCreatureType t){
    switch(t){
        case AquariumCreatureType::BiggerFish:
            return "BiggerFish";
        case AquariumCreatureType::NPCreature:
            return "BaseFish";
        case AquariumCreatureType::SwordFish:
            return "SwordFish";
        case AquariumCreatureType::Eel:
            return "Eel";
        default:
            return "UknownFish";
    }
}

// PlayerCreature Implementation
PlayerCreature::PlayerCreature(float x, float y, int speed, std::shared_ptr<GameSprite> sprite)
: Creature(x, y, speed, 10.0f, 1, sprite) {}


void PlayerCreature::setDirection(float dx, float dy) {
    m_dx = dx;
    m_dy = dy;
    normalize();
}

void PlayerCreature::move() {
    m_x += m_dx * m_speed;
    m_y += m_dy * m_speed;
    this->bounce();
}

void PlayerCreature::reduceDamageDebounce() {
    if (m_damage_debounce > 0) {
        --m_damage_debounce;
    }
}

void PlayerCreature::update() {
    this->reduceDamageDebounce();
    this->move();
}


void PlayerCreature::draw() const {
    
    ofLogVerbose() << "PlayerCreature at (" << m_x << ", " << m_y << ") with speed " << m_speed << std::endl;
    ofPushStyle(); 
    ofPushMatrix(); 

    float scale = 1.0f;
    if (m_powered) {
        float pulse = sin(ofGetElapsedTimef() * 5.0f) * 0.2f; 
        scale += pulse;
    }

    
    float cx = m_x;
    float cy = m_y;
    ofTranslate(cx, cy);
    ofScale(scale, scale);
    ofTranslate(-cx, -cy);

    if (this->m_damage_debounce > 0) {
        ofSetColor(ofColor::red); // Flash red if in damage debounce 
    } 
    
    // Rainbow glow when powered
    else if (m_powered) {
        float t = ofGetElapsedTimef() * 2.0f; 
        ofColor rainbow = ofColor::fromHsb(fmod(t * 50, 255), 200, 255); 
        ofSetColor(rainbow);
    } 
    else {
        ofSetColor(ofColor::white);// restore color
    }

    
    if (m_sprite) {
        m_sprite->draw(m_x, m_y);

        //tag to notice player
        ofSetColor(ofColor::white);
        string tag = "PLAYER";
        ofDrawBitmapString(tag, m_x + 5, m_y - 10);
    }

    ofPopMatrix(); // restore transform
    ofPopStyle();  

}

void PlayerCreature::changeSpeed(int speed) {
    m_speed = speed;
}

void PlayerCreature::loseLife(int debounce) {
    if (m_damage_debounce <= 0) {
        if (m_lives > 0) this->m_lives -= 1;
        m_damage_debounce = debounce; // Set debounce frames
        ofLogNotice() << "Player lost a life! Lives remaining: " << m_lives << std::endl;
    }
    // If in debounce period, do nothing
    if (m_damage_debounce > 0) {
        ofLogVerbose() << "Player is in damage debounce period. Frames left: " << m_damage_debounce << std::endl;
    }
}

// NPCreature Implementation
NPCreature::NPCreature(float x, float y, int speed, std::shared_ptr<GameSprite> sprite)
: Creature(x, y, speed, 30, 1, sprite) {
    m_dx = (rand() % 3 - 1); // -1, 0, or 1
    m_dy = (rand() % 3 - 1); // -1, 0, or 1
    normalize();

    m_creatureType = AquariumCreatureType::NPCreature;
}

void NPCreature::move() {
    // Simple AI movement logic (random direction)
    m_x += m_dx * m_speed;
    m_y += m_dy * m_speed;
    if(m_dx < 0 ){
        this->m_sprite->setFlipped(true);
    }else {
        this->m_sprite->setFlipped(false);
    }
    bounce();
}

void NPCreature::draw() const {
    ofLogVerbose() << "NPCreature at (" << m_x << ", " << m_y << ") with speed " << m_speed << std::endl;
    ofSetColor(ofColor::white);
    if (m_sprite) {
        m_sprite->draw(m_x, m_y);
    }
}


BiggerFish::BiggerFish(float x, float y, int speed, std::shared_ptr<GameSprite> sprite)
: NPCreature(x, y, speed, sprite) {
    m_dx = (rand() % 3 - 1);
    m_dy = (rand() % 3 - 1);
    normalize();

    setCollisionRadius(60); // Bigger fish have a larger collision radius
    m_value = 5; // Bigger fish have a higher value
    m_creatureType = AquariumCreatureType::BiggerFish;
}

void BiggerFish::move() {
    // Bigger fish might move slower or have different logic
    m_x += m_dx * (m_speed * 0.5); // Moves at half speed
    m_y += m_dy * (m_speed * 0.5);
    if(m_dx < 0 ){
        this->m_sprite->setFlipped(true);
    }else {
        this->m_sprite->setFlipped(false);
    }

    bounce();
}

void BiggerFish::draw() const {
    ofLogVerbose() << "BiggerFish at (" << m_x << ", " << m_y << ") with speed " << m_speed << std::endl;
    this->m_sprite->draw(this->m_x, this->m_y);
}

class SwordFish : public NPCreature{
public:
    SwordFish(float x,float y,int speed,std::shared_ptr<GameSprite> sprite)
    : NPCreature(x,y,speed,sprite){
        setCollisionRadius(25);
        m_value = 4;
        m_creatureType = AquariumCreatureType::SwordFish;
        m_dx = 1.0f; m_dy = 0.0f; normalize();
    }
    void move() override {
        if (m_dashing) {
            if (m_timer % 8 == 0) {
                m_dx = (rand()%2 ? 1.f : -1.f);
                m_dy = (rand()%3 - 1) * 0.4f;
                normalize();
            }
            m_x += m_dx * (m_speed * 1.5f);
            m_y += m_dy * (m_speed * 1.5f);
            if (++m_timer >= 28) { m_timer = 0; m_dashing = false; }
        } else {
            if (m_timer == 0) { m_dx = 0.25f; m_dy = 0.0f; }
            m_x += m_dx * (m_speed * 0.5f);
            m_y += m_dy * (m_speed * 0.5f);
            if (++m_timer >= 14) { m_timer = 0; m_dashing = true; }
        }
        if (m_sprite) m_sprite->setFlipped(m_dx < 0);
        bounce();
    }
private:
    int  m_timer   = 0;
    bool m_dashing = true;
};
class Eel : public NPCreature {
public:
    Eel(float x,float y,int speed,std::shared_ptr<GameSprite> sprite)
    : NPCreature(x,y,speed,sprite){
        setCollisionRadius(26);
        m_value = 3;
        m_creatureType = AquariumCreatureType::Eel;
        m_dx = 0.8f; m_dy = 0.0f; normalize();
    }
    void move() override {
        m_t += 0.04f;
        float wave = sinf(m_t) * 0.9f;
        m_dx = (m_dx >= 0 ? 1.0f : -1.0f);
        m_dy = wave;
        normalize();
        m_x += m_dx * (m_speed * 1.1f);
        m_y += m_dy * (m_speed * 1.1f);
        if (m_sprite) m_sprite->setFlipped(m_dx < 0);
        bounce();
    }
private:
    float m_t = 0.f;
};


// AquariumSpriteManager
AquariumSpriteManager::AquariumSpriteManager(){
    this->m_npc_fish = std::make_shared<GameSprite>("bluetang.png", 70,70);
    this->m_big_fish = std::make_shared<GameSprite>("Moorish.png", 120, 120);
    this->m_swordfish = std::make_shared<GameSprite>("swordfish.png", 140, 60);
    this->m_eel = std::make_shared<GameSprite>("eel.png", 120, 40);
    this->m_player_fish = std::make_shared<GameSprite>("clownfish.png", 70,70);

}

std::shared_ptr<GameSprite> AquariumSpriteManager::GetSprite(AquariumCreatureType t){
    switch(t){
        case AquariumCreatureType::BiggerFish:
            return std::make_shared<GameSprite>("Moorish.png", 120, 120);
        case AquariumCreatureType::NPCreature:
            return std::make_shared<GameSprite>("bluetang.png", 70, 70);
        case AquariumCreatureType::SwordFish:
            return std::make_shared<GameSprite>("swordfish.png", 140, 60);

        case AquariumCreatureType::Eel:
            return std::make_shared<GameSprite>("eel.png", 120, 40);
        default:
            return nullptr;
    }
}


// Aquarium Implementation
Aquarium::Aquarium(int width, int height, std::shared_ptr<AquariumSpriteManager> spriteManager)
    : m_width(width), m_height(height) {
        m_sprite_manager =  spriteManager;
    }



void Aquarium::addCreature(std::shared_ptr<Creature> creature) {
    creature->setBounds(m_width - 20, m_height - 20);
    m_creatures.push_back(creature);
}

void Aquarium::addAquariumLevel(std::shared_ptr<AquariumLevel> level){
    if(level == nullptr){return;} // guard to not add noise
    this->m_aquariumlevels.push_back(level);
}

void Aquarium::update() {
    for (auto& creature : m_creatures) {
        creature->move();
    }
    this->Repopulate();
}

void Aquarium::draw() const {
    for (const auto& creature : m_creatures) {
        creature->draw();
    }
}


void Aquarium::removeCreature(std::shared_ptr<Creature> creature) {
    auto it = std::find(m_creatures.begin(), m_creatures.end(), creature);
    if (it != m_creatures.end()) {
        ofLogVerbose() << "removing creature " << endl;
        int selectLvl = this->currentLevel % this->m_aquariumlevels.size();
        auto npcCreature = std::static_pointer_cast<NPCreature>(creature);
        this->m_aquariumlevels.at(selectLvl)->ConsumePopulation(npcCreature->GetType(), npcCreature->getValue());
        m_creatures.erase(it);
    }
}

void Aquarium::clearCreatures() {
    m_creatures.clear();
}

std::shared_ptr<Creature> Aquarium::getCreatureAt(int index) {
    if (index < 0 || size_t(index) >= m_creatures.size()) {
        return nullptr;
    }
    return m_creatures[index];
}

bool Aquarium::checkCollision(const std::shared_ptr<Creature>& a,
                              const std::shared_ptr<Creature>& b){
    if (!a || !b) return false;

    const float dx = a->getX() - b->getX();
    const float dy = a->getY() - b->getY();
    const float r  = a->getCollisionRadius() + b->getCollisionRadius();
    return (dx*dx + dy*dy) <= (r*r);
}

void Aquarium::handleCollision(const std::shared_ptr<Creature>& a,
                               const std::shared_ptr<Creature>& b)
{
    if (a) a->bounce();
    if (b) b->bounce();
}

void Aquarium::SpawnCreature(AquariumCreatureType type) {
    int x = rand() % this->getWidth();
    int y = rand() % this->getHeight();
    int speed = 1 + rand() % 25; // Speed between 1 and 25



    // Pick a radius by type using simple ifs:
    float radius = 30.0f; //defail - small fish
    if (type == AquariumCreatureType::BiggerFish) {
        radius = 60.0f;   //big fish
    }
    if (type == AquariumCreatureType::SwordFish) {
        radius = 28.0f; 
    }

    if (type == AquariumCreatureType::Eel){
        radius = 28.0f;
    }

    // Keep the spawn point inside the glass
    if (x < (int)radius) x = (int)radius;
    if (x > this->getWidth()  - (int)radius) x = this->getWidth()  - (int)radius;
    if (y < (int)radius) y = (int)radius;
    if (y > this->getHeight() - (int)radius) y = this->getHeight() - (int)radius;



    switch (type) {
        case AquariumCreatureType::NPCreature:
            this->addCreature(std::make_shared<NPCreature>(x, y, speed, this->m_sprite_manager->GetSprite(AquariumCreatureType::NPCreature)));
            break;
        case AquariumCreatureType::BiggerFish:
            this->addCreature(std::make_shared<BiggerFish>(x, y, speed, this->m_sprite_manager->GetSprite(AquariumCreatureType::BiggerFish)));
            break;
        case AquariumCreatureType::SwordFish:
            this->addCreature(std::make_shared<SwordFish>(x, y, speed, this->m_sprite_manager->GetSprite(AquariumCreatureType::SwordFish)));
            break;
        case AquariumCreatureType::Eel:
            this->addCreature(std::make_shared<Eel>(x, y, speed, this->m_sprite_manager->GetSprite(AquariumCreatureType::Eel)));
            break;
            default:
            ofLogError() << "Unknown creature type to spawn!";
            break;
    }

}


// repopulation will be called from the levl class
// it will compose into aquarium so eating eats frm the pool of NPCs in the lvl class
// once lvl criteria met, we move to new lvl through inner signal asking for new lvl
// which will mean incrementing the buffer and pointing to a new lvl index
void Aquarium::Repopulate() {
    ofLogVerbose("entering phase repopulation");
    // lets make the levels circular
    int selectedLevelIdx = this->currentLevel % this->m_aquariumlevels.size();
    ofLogVerbose() << "the current index: " << selectedLevelIdx << endl;
    std::shared_ptr<AquariumLevel> level = this->m_aquariumlevels.at(selectedLevelIdx);


    if(level->isCompleted()){
        level->levelReset();
        this->currentLevel += 1;
        selectedLevelIdx = this->currentLevel % this->m_aquariumlevels.size();
        ofLogNotice()<<"new level reached : " << selectedLevelIdx << std::endl;
        level = this->m_aquariumlevels.at(selectedLevelIdx);
        this->clearCreatures();
    }

    
    // now lets find how many to respawn if needed 
    std::vector<AquariumCreatureType> toRespawn = level->Repopulate();
    ofLogVerbose() << "amount to repopulate : " << toRespawn.size() << endl;
    if(toRespawn.size() <= 0 ){return;} // there is nothing for me to do here
    for(AquariumCreatureType newCreatureType : toRespawn){
        this->SpawnCreature(newCreatureType);
    }
}


// Aquarium collision detection
std::shared_ptr<GameEvent> DetectAquariumCollisions(std::shared_ptr<Aquarium> aquarium, std::shared_ptr<PlayerCreature> player) {
    if (!aquarium || !player) return nullptr;
    
    for (int i = 0; i < aquarium->getCreatureCount(); ++i) {
        std::shared_ptr<Creature> npc = aquarium->getCreatureAt(i);
        if (!npc || npc.get() == player.get()) continue;
        if (aquarium->checkCollision(player, npc)) {
            
            aquarium->handleCollision(player, npc);
            return std::make_shared<GameEvent>(GameEventType::COLLISION, player, npc);
        }
    }
    return nullptr;
};

//  Imlementation of the AquariumScene

void AquariumGameScene::Update(){
    //bubbles 
    spawnBubble();
    std::shared_ptr<GameEvent> event;

    this->m_player->update();

    if (this->updateControl.tick()) {
        event = DetectAquariumCollisions(this->m_aquarium, this->m_player);
        if (event != nullptr && event->isCollisionEvent()) {
            ofLogVerbose() << "Collision detected between player and NPC!" << std::endl;
            if(event->creatureB != nullptr){
                event->print();
                if(this->m_player->getPower() < event->creatureB->getValue()){
                    ofLogNotice() << "Player is too weak to eat the creature!" << std::endl;
                    hurtSound.play();
                    this->m_player->loseLife(3*60); // 3 frames debounce, 3 seconds at 60fps
                    if(this->m_player->getLives() <= 0){
                        this->m_lastEvent = std::make_shared<GameEvent>(GameEventType::GAME_OVER, this->m_player, nullptr);
                        return;
                    }
                }
                else{
                    this->m_aquarium->removeCreature(event->creatureB);
                    this->m_player->addToScore(1, event->creatureB->getValue());
                    
                    //eating sound
                    eatSound.play();

                    if (this->m_player->getScore() % 25 == 0){
                        this->m_player->increasePower(1);
                        ofLogNotice() << "Player power increased to " << this->m_player->getPower() << "!" << std::endl;
                    }
                    
                }
                
                

            } else {
                ofLogError() << "Error: creatureB is null in collision event." << std::endl;
            }
        }
        this->m_aquarium->update();
    }
    if (!powerUpActive && ofRandom(0, 1000) < 2) { // small chance to spawn
    powerUpX = ofRandom(50, ofGetWidth() - 50);
    powerUpY = ofRandom(50, ofGetHeight() - 50);
    powerUpBaseY = powerUpY;
    powerUpActive = true;
}

// Check if player collects the power-up
if (powerUpActive) {
    float dx = powerUpX - m_player->getX();
    float dy = powerUpY - m_player->getY();
    float distance = sqrt(dx * dx + dy * dy);
    float collisionBuffer = 25.0f;

    if (distance <= powerUpRadius + m_player->getCollisionRadius() + collisionBuffer) {
        powerUpActive = false;
        m_player->setPowered(true);  // turns effect
        powerUpStartTime = ofGetElapsedTimef();

        basePlayerSpeed = m_player->getSpeed();
        m_player->changeSpeed(basePlayerSpeed * 2.0); // gives speed
    }
}

// duration of power up
if (m_player->isPowered()) {
    float elapsed = ofGetElapsedTimef() - powerUpStartTime;
    
    if (elapsed > powerUpDuration) {
     
        // resets when power up is done
        m_player->setPowered(false);            
        m_player->changeSpeed(basePlayerSpeed); 
    }
 }
}

void AquariumGameScene::Draw() {
    this->m_player->draw();
    this->m_aquarium->draw();

    //drawing bubbles
    drawBubbles();
     // shows power up to collect
    if (powerUpActive) {
        float bobOffset = sin(ofGetElapsedTimef() * 2.0f) * 5.0f;
        ofSetColor(255, 255, 0, 200); 
        ofDrawCircle(powerUpX, powerUpBaseY + bobOffset, powerUpRadius);
        ofSetColor(255);
    }
     this->paintAquariumHUD();

}


void AquariumGameScene::paintAquariumHUD(){
    float panelWidth = ofGetWindowWidth() - 150;
    ofDrawBitmapString("Score: " + std::to_string(this->m_player->getScore()), panelWidth, 20);
    ofDrawBitmapString("Power: " + std::to_string(this->m_player->getPower()), panelWidth, 30);
    ofDrawBitmapString("Lives: " + std::to_string(this->m_player->getLives()), panelWidth, 40);
    for (int i = 0; i < this->m_player->getLives(); ++i) {
        ofSetColor(ofColor::red);
        ofDrawCircle(panelWidth + i * 20, 50, 5);
    }
    ofSetColor(ofColor::white); // Reset color to white for other drawings
}

void AquariumLevel::populationReset(){
    for(auto node: this->m_levelPopulation){
        node->currentPopulation = 0; // need to reset the population to ensure they are made a new in the next level
    }
}

void AquariumLevel::ConsumePopulation(AquariumCreatureType creatureType, int power){
    for(std::shared_ptr<AquariumLevelPopulationNode> node: this->m_levelPopulation){
        ofLogVerbose() << "consuming from this level creatures" << endl;
        if(node->creatureType == creatureType){
            ofLogVerbose() << "-cosuming from type: " << AquariumCreatureTypeToString(node->creatureType) <<" , currPop: " << node->currentPopulation << endl;
            if(node->currentPopulation == 0){
                return;
            } 
            node->currentPopulation -= 1;
            ofLogVerbose() << "+cosuming from type: " << AquariumCreatureTypeToString(node->creatureType) <<" , currPop: " << node->currentPopulation << endl;
            this->m_level_score += power;
            return;
        }
    }
}

std::vector<AquariumCreatureType> AquariumLevel::BuildRepopulateList() {
    std::vector<AquariumCreatureType> toRepopulate;
    for (std::shared_ptr<AquariumLevelPopulationNode> node : this->m_levelPopulation) {
        int delta = node->population - node->currentPopulation;
        if (delta > 0) {
            for (int i = 0; i < delta; ++i) {
                 toRepopulate.push_back(node->creatureType);
                  }
            node->currentPopulation += delta;
             }
    }
    return toRepopulate;
}

bool AquariumLevel::isCompleted(){
    return this->m_level_score >= this->m_targetScore;
}

std::vector<AquariumCreatureType> Level_0::Repopulate() {
    return BuildRepopulateList();
}

std::vector<AquariumCreatureType> Level_1::Repopulate() {
    return BuildRepopulateList();
}

std::vector<AquariumCreatureType> Level_2::Repopulate() {
    return BuildRepopulateList();
}//implementing new levels

std::vector<AquariumCreatureType> Level_3::Repopulate() {
    return BuildRepopulateList();
}

std::vector<AquariumCreatureType> Level_4::Repopulate() {
    return BuildRepopulateList();
}
