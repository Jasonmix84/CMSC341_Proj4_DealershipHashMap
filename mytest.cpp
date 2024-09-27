// CMSC 341 - Fall 2023 - Project 4
#include "dealer.h"
#include <random>
#include <vector>
#include <algorithm>

const char * FAIL_STATEMENT = "*****TEST FAILED: ";
const char * PASS_STATEMENT = "*****TEST PASSED: ";

enum RANDOM {UNIFORMINT, UNIFORMREAL, NORMAL, SHUFFLE};
class Random {
public:
    Random(int min, int max, RANDOM type=UNIFORMINT, int mean=50, int stdev=20) : m_min(min), m_max(max), m_type(type)
    {
        if (type == NORMAL){
            //the case of NORMAL to generate integer numbers with normal distribution
            m_generator = std::mt19937(m_device());
            //the data set will have the mean of 50 (default) and standard deviation of 20 (default)
            //the mean and standard deviation can change by passing new values to constructor 
            m_normdist = std::normal_distribution<>(mean,stdev);
        }
        else if (type == UNIFORMINT) {
            //the case of UNIFORMINT to generate integer numbers
            // Using a fixed seed value generates always the same sequence
            // of pseudorandom numbers, e.g. reproducing scientific experiments
            // here it helps us with testing since the same sequence repeats
            m_generator = std::mt19937(10);// 10 is the fixed seed value
            m_unidist = std::uniform_int_distribution<>(min,max);
        }
        else if (type == UNIFORMREAL) { //the case of UNIFORMREAL to generate real numbers
            m_generator = std::mt19937(10);// 10 is the fixed seed value
            m_uniReal = std::uniform_real_distribution<double>((double)min,(double)max);
        }
        else { //the case of SHUFFLE to generate every number only once
            m_generator = std::mt19937(m_device());
        }
    }
    void setSeed(int seedNum){
        // we have set a default value for seed in constructor
        // we can change the seed by calling this function after constructor call
        // this gives us more randomness
        m_generator = std::mt19937(seedNum);
    }

    void getShuffle(vector<int> & array){
        // the user program creates the vector param and passes here
        // here we populate the vector using m_min and m_max
        for (int i = m_min; i<=m_max; i++){
            array.push_back(i);
        }
        shuffle(array.begin(),array.end(),m_generator);
    }

    void getShuffle(int array[]){
        // the param array must be of the size (m_max-m_min+1)
        // the user program creates the array and pass it here
        vector<int> temp;
        for (int i = m_min; i<=m_max; i++){
            temp.push_back(i);
        }
        std::shuffle(temp.begin(), temp.end(), m_generator);
        vector<int>::iterator it;
        int i = 0;
        for (it=temp.begin(); it != temp.end(); it++){
            array[i] = *it;
            i++;
        }
    }

    int getRandNum(){
        // this function returns integer numbers
        // the object must have been initialized to generate integers
        int result = 0;
        if(m_type == NORMAL){
            //returns a random number in a set with normal distribution
            //we limit random numbers by the min and max values
            result = m_min - 1;
            while(result < m_min || result > m_max)
                result = m_normdist(m_generator);
        }
        else if (m_type == UNIFORMINT){
            //this will generate a random number between min and max values
            result = m_unidist(m_generator);
        }
        return result;
    }

    double getRealRandNum(){
        // this function returns real numbers
        // the object must have been initialized to generate real numbers
        double result = m_uniReal(m_generator);
        // a trick to return numbers only with two deciaml points
        // for example if result is 15.0378, function returns 15.03
        // to round up we can use ceil function instead of floor
        result = std::floor(result*100.0)/100.0;
        return result;
    }
    
    private:
    int m_min;
    int m_max;
    RANDOM m_type;
    std::random_device m_device;
    std::mt19937 m_generator;
    std::normal_distribution<> m_normdist;//normal distribution
    std::uniform_int_distribution<> m_unidist;//integer uniform distribution
    std::uniform_real_distribution<double> m_uniReal;//real uniform distribution

};
class Tester{
    public:
    unsigned int hashCode(const string str);

    string carModels[5] = {"challenger", "stratos", "gt500", "miura", "x101"};
    string dealers[5] = {"super car", "mega car", "car world", "car joint", "shack of cars"};

    void makeTable(CarDB& cardb, int insertNum, bool& test){
        Random RndID(MINID,MAXID);
        Random RndCar(0,4);// selects one from the carModels array
        Random RndQuantity(0,50);
        
        for (int i=0;i<insertNum;i++){
            // generating random data
            Car dataObj = Car(carModels[RndCar.getRandNum()], RndQuantity.getRandNum(), 
                            RndID.getRandNum());
            // inserting data in to the CarDB object
            
            if (test && cardb.insert(dataObj)) {
                test = true;
            }
            else{
                test = false;
            }
        }
    }

};

unsigned int hashCode(const string str);

string carModels[5] = {"challenger", "stratos", "gt500", "miura", "x101"};
string dealers[5] = {"super car", "mega car", "car world", "car joint", "shack of cars"};

int main(){
    Tester tester;

    ////////////////////INSERT TESTING//////////////////////
    {
        Random RndID(MINID,MAXID);
        Random RndQuantity(0,50);
        //nomal case
        bool passed = true;
        CarDB cardb(MINPRIME, hashCode, DOUBLEHASH);
        for (int i=0; i<4; i++){
            Car dataObj = Car(carModels[i], RndQuantity.getRandNum(), RndID.getRandNum());
            if (passed && cardb.insert(dataObj)){
                passed = true;
            }
            else{
                passed = false;
            }
        }
        cout << "Normal Case for Insetion Testing no collisions: ";
        
        
        if (passed){
            cout  << PASS_STATEMENT << endl;
        }
        else{
            cout << FAIL_STATEMENT << endl;
        }
        cout << endl;
    }

    {
        //edge case
        bool passed = true;
        CarDB cardb(MINPRIME, hashCode, DOUBLEHASH);
        tester.makeTable(cardb, 49, passed);
        cout << "Edge Case testing multiple collisions for Insetion Testing: ";
        if (passed){
            cout  << PASS_STATEMENT << endl;
        }
        else{
            cout << FAIL_STATEMENT << endl;
        }
        cout << endl;
    }

    {
        //test collision with NONE collision policy
        //edge case
        bool passed = true;
        CarDB cardb(MINPRIME, hashCode, NONE);
        tester.makeTable(cardb, 49, passed);
        cout << "Edge Case testing collision  Insetion Testing Pass occurs if some Nodes are not inserted: ";
        if (!passed){
            cout  << PASS_STATEMENT << endl;
        }
        else{
            cout << FAIL_STATEMENT << endl;
        }
        cout << endl;
    }
    
    {
        //rehash normal case
        bool passed = true;
        CarDB cardb(MINPRIME, hashCode, DOUBLEHASH);
        tester.makeTable(cardb, 51, passed);
        cout << "Edge Case testing multiple collisions for Insetion Testing: ";
        if (passed){
            cout  << PASS_STATEMENT << endl;
        }
        else{
            cout << FAIL_STATEMENT << endl;
        }
        cout << endl;
    }

    {
        //inserting duplicate
        bool passed = true;
        CarDB cardb(MINPRIME, hashCode, DOUBLEHASH);
        Car car1 = Car(carModels[0], 2, MINID);
        //Car car2 = Car(carModels[0], 4, MINID);
        if (passed && cardb.insert(car1)){
            passed = true;
        }
        else{
            passed = false;
        }
        if (passed && !cardb.insert(car1)){
            passed = true;
        }
        else{
            passed = false;
        }
        
        cout << "Error Case test for duplicate insertions passes if second does not get added: ";
        
        if (passed){
            cout  << PASS_STATEMENT << endl;
        }
        else{
            cout << FAIL_STATEMENT << endl;
        }
        cout << endl;
    }

    {
        //inserting with Dealer ID out of range
        bool passed = true;
        CarDB cardb(MINPRIME, hashCode, DOUBLEHASH);
        Car car1 = Car(carModels[0], 2, 0);
        //Car car2 = Car(carModels[0], 4, MINID);
        if (passed && !cardb.insert(car1)){
            passed = true;
        }
        else{
            passed = false;
        }

        cout << "Error Case test for dealerID outside of range passes if car does not get added: ";
        
        if (passed){
            cout  << PASS_STATEMENT << endl;
        }
        else{
            cout << FAIL_STATEMENT << endl;
        }
        cout << endl;
    
    }



    ////////////////////REMOVE TESTING//////////////////////   
    {
        //normal case one remove
        Random RndID(MINID,MAXID);
        Random RndQuantity(0,50);
        bool inserted = true;
        CarDB cardb(MINPRIME, hashCode, DOUBLEHASH);
        vector<Car> dataList;
        for (int i=0; i<4; i++){
            Car dataObj = Car(carModels[i], RndQuantity.getRandNum(), MINID);

            dataList.push_back(dataObj);
            
            if (inserted && cardb.insert(dataObj)){
                inserted = true;
            }
            else{
                inserted = false;
            }
        }

        cout << "Testing 1 remove on a CarDB passes if successfully deletes: ";
        if (cardb.remove(dataList.at(0))){
            cout << PASS_STATEMENT << endl;
        }
        else{
            cout << FAIL_STATEMENT << endl;
        }
        cout << endl;
    }
    
    {
        //remove all causing a rehash
        vector<Car> dataList;
        Random RndID(MINID,MAXID);
        Random RndCar(0,4);// selects one from the carModels array
        Random RndQuantity(0,50);
        CarDB cardb(MINPRIME, hashCode, DOUBLEHASH);
        bool passed = true;
        
        for (int i=0;i<50;i++){
            // generating random data
            Car dataObj = Car(carModels[RndCar.getRandNum()], RndQuantity.getRandNum(), 
                            RndID.getRandNum());
            // saving data for later use
            dataList.push_back(dataObj);
            // inserting data in to the CarDB object
            if (passed && cardb.insert(dataObj)){
                passed = true;
            }
            else{
                passed = false;
            }
        }
        for (int i=0; i<40; i++){
            //do 5 deletions deleted ratio > .8
            if (passed && cardb.remove(dataList.at(i))){
                passed =  true;
            }
            else{
                passed = false;
            }
        }
        cout << "Testing rehash caused by deleted ratio being greater than 80%: ";
        if (passed){
            cout << PASS_STATEMENT << endl;
        }
        else{
            cout << FAIL_STATEMENT << endl;
        }

        cout << endl;
    }

    {
        //delete an element that doesnt exist
        bool passed = true;
        CarDB cardb(MINPRIME, hashCode, DOUBLEHASH);
        Car car1 = Car(carModels[0], 2, MINID);
        Car car2 = Car(carModels[0], 2, MINID + 1);
        //Car car2 = Car(carModels[0], 4, MINID);
        if (cardb.insert(car1)){
            passed = true;
        }
        else{
            passed = false;
        }

        if (passed && !cardb.remove(car2)){
            passed = true;
        }
        else{
            passed = false;
        }

        cout << "Error Case for remove, passes if trying to remove a car that is not in the DB returns false: ";
    
        if (passed){
            cout  << PASS_STATEMENT << endl;
        }
        else{
            cout << FAIL_STATEMENT << endl;
        }
        cout << endl;
    
    }
 

    ////////////////////getCar TESTING////////////////////// 
    {
        //get car normal case  
        bool passed = true;
        CarDB cardb(MINPRIME, hashCode, QUADRATIC);
        tester.makeTable(cardb, 25, passed);
        Car myCar = Car("challenger", 46, MAXID);
        cardb.insert(myCar);

        cout << "Testing getCar on a large data set: ";
        if (cardb.getCar("challenger",MAXID) == myCar){
            cout << PASS_STATEMENT << endl;
        }
        else {
            cout << FAIL_STATEMENT  << endl;
        }
        cout << endl;
    }

    {
        //get car on large set  
        bool passed = true;
        CarDB cardb(MAXPRIME, hashCode, QUADRATIC);
        tester.makeTable(cardb, 49990, passed);
        Car myCar = Car("challenger", 100, MAXID);
        cardb.insert(myCar);

        cout << "Testing getCar on a large data set: ";
        if (cardb.getCar("challenger",MAXID) == myCar){
            cout << PASS_STATEMENT << endl;
        }
        else {
            cout << FAIL_STATEMENT  << endl;
        }
        cout << endl;
    }

    {
        //call on a car that does not exist
        //get car on large set  
        bool passed = true;
        CarDB cardb(MAXPRIME, hashCode, QUADRATIC);
        tester.makeTable(cardb, 49990, passed);
        Car myCar = Car("challenger", 100, MAXID);

        // cardb.insert(myCar); dont insert

        cout << "Testing getCar on a large data set: ";
        //returns and empty car
        if (cardb.getCar("challenger",MAXID) == Car()){
            cout << PASS_STATEMENT << endl;
        }
        else {
            cout << FAIL_STATEMENT  << endl;
        }
        cout << endl;
    }

    {
        //getcar on a car in old table
        bool passed = true;
        CarDB cardb(MINPRIME, hashCode, DOUBLEHASH);
        tester.makeTable(cardb, 50, passed);
        Car myCar = Car("challenger", 100, MAXID);

        cardb.insert(myCar);
        cout << "calling getCar on a car in old table: ";

        if (cardb.getCar("challenger",MAXID) == myCar){
            cout << PASS_STATEMENT << endl;
        }
        else {
            cout << FAIL_STATEMENT  << endl;
        }
        cout << endl;
    }

    ////////////////////updateQuantity TESTING//////////////////////
    {
        //normal call of updateQuantity
        bool passed = true;
        CarDB cardb(MAXPRIME, hashCode, QUADRATIC);
        tester.makeTable(cardb, 49990, passed);
        Car myCar = Car("challenger", 100, MAXID);
        Car resultingCar = Car("challenger", 1, MAXID);
        cardb.insert(myCar);

        cout << "Testing Normal case of updateQuantity: ";
        if (cardb.updateQuantity(myCar, 1)){
            passed = true;
        }
        else {
            passed = false;
        }

        if (passed && cardb.getCar("challenger", MAXID) == resultingCar){
            cout << PASS_STATEMENT << endl;
        }
        else{
            cout << FAIL_STATEMENT << endl;
        }
        cout << endl;
    }
    
    
    {
        //update quantity on car that doesnt exist
        //get car on large set  
        bool passed = true;
        CarDB cardb(MAXPRIME, hashCode, QUADRATIC);
        tester.makeTable(cardb, 49990, passed);
        Car myCar = Car("challenger", 100, MAXID);

        // cardb.insert(myCar); dont insert

        cout << "Testing getCar on a large data set: ";
        //should return false so it passes if it returns false
        if (cardb.updateQuantity(myCar, 1)){
            cout << FAIL_STATEMENT << endl;
        }
        else {
            cout << PASS_STATEMENT  << endl;
        }
        cout << endl;
    
    }
    ////////////////////changeProbPolicy TESTING//////////////////////
    {
        bool passed = true;
        CarDB cardb(MINPRIME, hashCode, QUADRATIC);
        tester.makeTable(cardb, 50, passed);
        cardb.changeProbPolicy(DOUBLEHASH);
        Car myCar = Car("challenger", 100, MAXID);
        Car myCar2 = Car("challenger", 100, MINID);
        passed = cardb.insert(myCar);

        cout << "Testing that rehash works after changing probing policy: ";
        if (passed && cardb.insert(myCar2)){
            cout << PASS_STATEMENT << endl;
        }
        else{
            cout << FAIL_STATEMENT << endl;
        }

    }

}
unsigned int hashCode(const string str) {
   unsigned int val = 0 ;
   const unsigned int thirtyThree = 33 ;  // magic number from textbook
   for (unsigned int i = 0 ; i < str.length(); i++)
      val = val * thirtyThree + str[i] ;
   return val ;
}