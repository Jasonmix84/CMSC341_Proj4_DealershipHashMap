// CMSC 341 - Fall 2023 - Project 4
#include "dealer.h"
CarDB::CarDB(int size, hash_fn hash, prob_t probing = DEFPOLCY)
: m_hash(hash), m_currProbing(probing) {
    //The table size must be a prime number between MINPRIME and MAXPRIME. 
    //If the user passes a size less than MINPRIME, the capacity must be set to MINPRIME
    //If the user passes a size larger than MAXPRIME, the capacity must be set to MAXPRIME. 
    //If the user passes a non-prime number the capacity must be set to the smallest prime number greater than user's value.
    if (size < MINPRIME){
        m_currentCap = MINPRIME;
    }
    else if (size > MAXPRIME){
        m_currentCap = MAXPRIME;
    }
    else if (!isPrime(size)){
        m_currentCap = findNextPrime(size);
    }
    else{
        m_currentCap = size;
    }

    //initialize tables
    m_currentTable = new Car[m_currentCap];
    for (int i = 0; i < m_currentCap; i++){
        m_currentTable[i] = Car();
    }

    m_currentSize = 0;
    m_currNumDeleted = 0;


    
    m_oldTable = nullptr;
    m_oldCap = 0;
    m_oldSize = 0;
    m_oldNumDeleted = 0;
    m_oldProbing = probing;
    m_newPolicy = probing;
}

CarDB::~CarDB(){
    if (m_currentTable != nullptr){
        delete[] m_currentTable;
    }
    if (m_oldTable != nullptr){
        delete[] m_oldTable;
    }
    m_currentTable = nullptr;
    m_currentCap = 0;
    m_currentSize = 0;
    m_currNumDeleted = 0;
    m_oldTable = nullptr;
    m_oldCap = 0;
    m_oldSize = 0;
    m_oldNumDeleted = 0;
}

void CarDB::changeProbPolicy(prob_t policy){
    m_newPolicy = policy;
    
}

bool CarDB::insert(Car car){
    int index = 0;
    
    int count = 0;
    bool inserted = false;

    //Check that Car ID is valid
    if (car.m_dealer < MINID || car.m_dealer > MAXID){
        return false;
    }
    //get index using hash function
    index =  m_hash(car.m_model) % m_currentCap;
    //check if the index is used 
    if (m_currentTable[index].m_model == ""){
        // //check for duplicates
        if (m_currentTable[index].m_model == car.m_model && m_currentTable[index].m_dealer == car.m_dealer){
            return false;
        }

        m_currentTable[index] = car;
        m_currentSize++;
        inserted = true;
    }
    else{
        if (rehash(index, car, "insert", m_currentCap, m_currProbing)){
            inserted = true;
        }
        else{

            inserted = false;
        }
    }
    if ((lambda() > 0.5) && m_oldTable == nullptr){
        //check if first part of rehash is necessary 
        inserted = quarterRehash(count);

    }
    else if ((m_oldTable != nullptr) && (((float) m_currentSize / (float) m_oldCap) > 0.25)){ 
        //finish rehash
        inserted = finishRehash(inserted);
        delete[] m_oldTable;
        m_oldTable = nullptr;
    }
    return inserted;
}

bool CarDB::remove(Car car){
    //HAVE NOT TESTED YET
    bool removed = false;
    int count = 0;
    if(getCar(car.m_model, car.m_dealer).m_model != ""){
        //if get car returns a non empty car mark as deleted
        car.m_used = true;
        m_currNumDeleted++;
        removed = true;
    }

    if ((deletedRatio() > .8) && m_oldTable == nullptr){
        //start rehash is necessary
        removed = quarterRehash(count);

    }
    else if ((m_oldTable != nullptr) && (((float) m_currentSize / (float) m_oldCap) < 0.25)){
        //finish rehash
        removed = finishRehash(removed);
        delete[] m_oldTable;
        m_oldTable = nullptr;
    }

    return removed;
    
}

Car CarDB::getCar(string model, int dealer) const{
    //if there is no table return an empty object
    if (m_currentTable == nullptr){
        return Car();
    }
    int currTableI =  m_hash(model) % m_currentCap;
    if(search(m_currentTable, currTableI, m_currentCap, m_currProbing, model, dealer).m_model != ""){
        //normal case of searching in the current table
        return search(m_currentTable, currTableI, m_currentCap, m_currProbing, model, dealer);
    }
    
    if(m_oldTable != nullptr){
        //search old table
        int oldTableI = m_hash(model) % m_oldCap;

        if(search(m_oldTable, oldTableI, m_oldCap, m_oldProbing, model, dealer).m_model != ""){
            return search(m_oldTable, oldTableI, m_oldCap, m_oldProbing, model, dealer);
        }
        else{
            return Car();
        }
        
    }
    //neither search return a car so return an empty car
    return Car();


}

float CarDB::lambda() const {
    //size accounts for live and deletes
    return (float) m_currentSize / (float) m_currentCap;
}

float CarDB::deletedRatio() const {
    return (float) m_currNumDeleted / (float) m_currentSize;
}

void CarDB::dump() const {
    cout << "Dump for the current table: " << endl;
    if (m_currentTable != nullptr)
        for (int i = 0; i < m_currentCap; i++) {
            cout << "[" << i << "] : " << m_currentTable[i] << endl;
        }
    cout << "Dump for the old table: " << endl;
    if (m_oldTable != nullptr)
        for (int i = 0; i < m_oldCap; i++) {
            cout << "[" << i << "] : " << m_oldTable[i] << endl;
        }
}

bool CarDB::updateQuantity(Car car, int quantity){
    //not tested
    bool updated = false;
    if (getCar(car.m_model, car.m_dealer).m_model != ""){
        Car foundCar = getCar(car.m_model, car.m_dealer);
        foundCar.m_quantity = quantity;
        updated = true;
    }
    return updated;
}

bool CarDB::isPrime(int number){
    bool result = true;
    for (int i = 2; i <= number / 2; ++i) {
        if (number % i == 0) {
            result = false;
            break;
        }
    }
    return result;
}

int CarDB::findNextPrime(int current){
    //we always stay within the range [MINPRIME-MAXPRIME]
    //the smallest prime starts at MINPRIME
    if (current < MINPRIME) current = MINPRIME-1;
    for (int i=current; i<MAXPRIME; i++) { 
        for (int j=2; j*j<=i; j++) {
            if (i % j == 0) 
                break;
            else if (j+1 > sqrt(i) && i != current) {
                return i;
            }
        }
    }
    //if a user tries to go over MAXPRIME
    return MAXPRIME;
}

ostream& operator<<(ostream& sout, const Car &car ) {
    if (!car.m_model.empty())
        sout << car.m_model << " (" << car.m_dealer << "," << car.m_quantity<< ")";
    else
        sout << "";
  return sout;
}

bool operator==(const Car& lhs, const Car& rhs){
    // since the uniqueness of an object is defined by model and delaer
    // the equality operator considers only those two criteria
    return ((lhs.m_model == rhs.m_model) && (lhs.m_dealer == rhs.m_dealer));
}

//HELPER FUNCTION
template <class T>
void CarDB::swap(T& x, T& y){
        //used in quarter rehash
        T temp = x;
        x = y;
        y = temp;
    }

bool CarDB::rehash(int index, Car car, string type, int cap, prob_t probe){
    int i = 1;
    bool inserted = false;
    bool notInserted = true;

    // //check for duplicates
    if (type == "insert"){
        if (m_currentTable[index].m_model == car.m_model && m_currentTable[index].m_dealer == car.m_dealer){
            return false;
        }
    }
    else{
        if (m_oldTable[index].m_model == car.m_model && m_oldTable[index].m_dealer == car.m_dealer){
            return false;
        }
    }
    
    while (notInserted){
        //get a new possible index
        if (probe == DOUBLEHASH){
            //double hashing
            index = (((m_hash(car.m_model) % cap) + (i * (11-(m_hash(car.m_model) % 11)))) % cap);
        }
        else if (probe == QUADRATIC){
            //quadratic probing
            index = (((m_hash(car.m_model) % cap) + (i * i)) % cap);
        }
        else{
            //NONE
            return false;
        }
        // //check for duplicates again
        if (type == "insert"){
            if (m_currentTable[index].m_model == car.m_model && m_currentTable[index].m_dealer == car.m_dealer){
                return false;
            }
        }
        else{
            if (m_oldTable[index].m_model == car.m_model && m_oldTable[index].m_dealer == car.m_dealer){
                return false;
            }
        }
        
        //check that index and if it is empty add it to the hash table and end loop
        if (type == "insert" && (m_currentTable[index].m_model == "")){
            //used when adding to current table
            m_currentTable[index] = car;
            m_currentSize++;
            inserted = true;
            notInserted = false;
        }
        else if (type == "rehash" && (m_oldTable[index].m_model == "")){
            //used in quarter rehash to add to old table which will then eventually become current table
            m_oldTable[index] = car;
            m_oldSize++;
            inserted = true;
            notInserted = false;
        }
        else{
            //if spot not available increase i and reprobe
            i += 1;
        }

        //After trying a certain amount of time giveup as insert is not possible
        if (i > m_currentCap){
            notInserted = false;
        }
    }
    return inserted;
}

bool CarDB::quarterRehash(int count){
    const int TWO = 2;
    const int FOUR = 4;
    int quarter = m_currentSize / TWO;
    int index = 0;
    bool rehashed = false;
    
    //create m_oldTable which is the smallest prime number greater than ((m_currentSize - m_numDeleted)*4).
    m_oldCap = ((m_currentSize - m_currNumDeleted)*FOUR);
    m_oldCap = findNextPrime(m_oldCap);
    
    //finish initializing
    m_oldTable = new Car[m_oldCap];
    for (int i = 0; i < m_oldCap; i++){
        m_oldTable[i] = Car();
    }

    for (int i = 0; i < m_currentCap; i++){
        //iterate until either 25% is transfered or until interated through entire capacity
        //only transfer nonused nodes and nonempty cars
        if ((m_currentTable[i].m_model != "") && (m_currentTable[i].m_used == false) && (count != quarter)){
            //get index using hash function
            index = m_hash(m_currentTable[i].m_model) % m_oldCap;// at this point old capicity is the newly created array size
            if (m_oldTable[index].m_model == ""){
                m_oldTable[index] = m_currentTable[i];
                m_oldSize++;
                //tag as deleted if transfered
                m_currentTable[i].m_used = true;
                m_currNumDeleted++;
                rehashed = true;
            }
            else{
                //reprobe to get new index
                if (rehash(index, m_currentTable[i], "rehash", m_oldCap, m_newPolicy)){
                    //tag transfered node as deleted
                    m_currentTable[i].m_used = true;
                    m_currNumDeleted++;
                    rehashed = true;
                }
                else{
                    //if insert not possible return false
                    return false;
                }
            }
            //set the m_model of table to "" like empty so they arent transfered again when finishing rehash
            m_currentTable[i].m_model = "";
            count++;
        }
    }
    //make the oldTable the current table and make the current the old
    swap(m_currentTable, m_oldTable);
    swap(m_currentCap, m_oldCap);
    swap(m_currentSize, m_oldSize);
    swap(m_currNumDeleted, m_oldNumDeleted);
    
    //make sure the old table matches with the old probing and current matches current table
    m_oldProbing = m_currProbing;
    m_currProbing = m_newPolicy;
    return rehashed;
}

bool CarDB::finishRehash(bool inserted){
    int index = 0;
    for (int i = 0; i < m_oldCap; i++){
        //transfer only non empty nodes and non used nodes
        if (m_oldTable[i].m_model != "" && (m_oldTable[i].m_used == false)){
            //get index and try it
            index = m_hash(m_oldTable[i].m_model) % m_currentCap;
            if (m_currentTable[index].m_model == ""){
                m_currentTable[index] = m_oldTable[i];
                m_currentSize++;
                //tag as deleted if transfered
                m_oldTable[i].m_used = true;
                m_oldNumDeleted++;

                //set the m_model of table to ""
                m_oldTable[i].m_model = "";
            }
            else{
                //original index didnt work so reprobe
                if (rehash(index, m_oldTable[i], "insert", m_currentCap, m_currProbing)){
                    m_currentTable[i].m_used = true;
                    m_currNumDeleted++;
                    inserted = true;
                }
                else{
                    return false;
                }
            }
            
        }    
    }
    return inserted;
}


Car CarDB::search(Car* table, int index, int cap, prob_t policy, string model, int dealer) const{
    int i = 1;
    //try first search
    if (table[index].m_model == model && table[index].m_dealer == dealer){
        return table[index];
    }
    else{
        while(i < cap){
            //reprobe stop when its not possible for node to exist
            if (policy == DOUBLEHASH){
                //double hashing
                index = (((m_hash(model) % cap) + (i * (11-(m_hash(model) % 11)))) % cap);
            }
            else if (policy == QUADRATIC){
                //quadratic probing
                index = ((m_hash(model) % cap) + (i * i)) % cap;
            }
            else{
                //none
                return Car();
            }

            //check if new index has the car we are searching for
            if (table[index].m_model == model && table[index].m_dealer == dealer){
                return table[index];
            }
            i++;
        }

    }
    //return empty car if not found
    return Car();
}