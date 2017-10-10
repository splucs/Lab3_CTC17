#include <bits/stdc++.h>
using namespace std;
#define MAX_ATRIBUTE 6
#define VALIDATIONSET_RATIO 0.05
#define MAX_MOVIES 4000

struct user {
	int id;
	char gender;
	int age;
	int occupation;
	int zipcode;
};
vector<user> users;

struct movie {
	int id;
	string genre;
	string title;
};
vector<movie> movies;

struct rating {
	int userid;
	int movieid;
	int value;
	int timestamp;
};
vector<rating> ratings;

struct block {
	int data[MAX_ATRIBUTE];
	int value;
};
vector<block> blocks;
typedef vector<block>::iterator pblock;

void ReadUsers() {
	printf("Reading users... ");
	
	//Open file
	ifstream input("users.dat");
	
	//Read until the end
	users.clear();
	string buffer;
	while(input) {
		
		//Read to buffer
		getline(input, buffer);
		int len = buffer.size();
		if (len < 8) break;
		
		//Parse info
		user current;
		sscanf(buffer.c_str(), "%d::%c::%d::%d::%d", &current.id, &current.gender, &current.age, &current.occupation, &current.zipcode);
		users.push_back(current);
		//printf("User read: %d %c %d %d %d\n", current.id, current.gender, current.age, current.occupation, current.zipcode);
	}
	
	printf("done: %u users read\n", users.size());

	//Close file
	input.close();
}

void ReadMovies() {
	printf("Reading movies... ");
	
	//Open file
	ifstream input("movies.dat");
	
	//Read until the end
	movies.clear();
	string buffer;
	while(input) {
		
		//Read to buffer
		getline(input, buffer);
		int len = buffer.size();
		if (len < 4) break;
		buffer[--len] = 0;
		
		//Parse info
		movie current;
		sscanf(buffer.c_str(), "%d::", &current.id);
		int st = 0, ed;
		
		while(buffer[st] != ':' || buffer[st+1] != ':') st++;
		st += 2;
		ed = st;
		while(buffer[ed] != ':' || buffer[ed+1] != ':') ed++;
		buffer[ed] = 0;
		ed += 2;
		current.title = string(&buffer[st]);
		current.genre = string(&buffer[ed]);
		
		movies.push_back(current);
		//printf("Movie read: %d title: %s genre: %s\n", current.id, current.title.c_str(), current.genre.c_str());
	}
	
	printf("done: %u movies read\n", movies.size());

	//Close file
	input.close();
}

void ReadRatings() {
	printf("Reading ratings... ");
	
	//Open file
	ifstream input("ratings.dat");
	
	//Read until the end
	ratings.clear();
	string buffer;
	while(input) {
		
		//Read to buffer
		getline(input, buffer);
		int len = buffer.size();
		if (len < 6) break;
		
		//Parse info
		rating current;
		sscanf(buffer.c_str(), "%d::%d::%d::%d", &current.userid, &current.movieid, &current.value, &current.timestamp);
		ratings.push_back(current);
		//printf("Rating read: %d %d %d %d\n", current.userid, current.movieid, current.value, current.timestamp);
	}
	
	printf("done: %u ratings read\n", ratings.size());

	//Close file
	input.close();
}

inline int ReduceGender(char gender) {
	if (gender == 'M' || gender == 'm') return 0;
	if (gender == 'F' || gender == 'f') return 1;
	return 2;
}

inline int ReduceAge(int age) {
	return age / 5;
}

inline int ReduceOccupation(int occupation) {
	return occupation;
}

inline int ReduceZipCode(int zipcode) {
	return zipcode / 100;
}

map<string, int> genre2id;
inline int ReduceGenre(string genre) {
	if (!genre2id.count(genre)) {
		int id = genre2id.size();
		genre2id[genre] = id;
	}
	return genre2id[genre];
}

inline int ReduceMovie(int movieid) {
	return movieid;
}

inline int ReduceTimestamp(int timestamp) {
	return timestamp / 10000000;
}

inline movie GetMovieById(int id) {
	
	//Do binary search
	int lo = 0;
	int hi = movies.size();
	while(hi > lo + 1) {
		int mid = (hi + lo) / 2;
		if (movies[mid].id <= id) lo = mid;
		else hi = mid;
	}
	return movies[lo];
}

inline user GetUserById(int id) {
	
	//Do binary search
	int lo = 0;
	int hi = users.size();
	while(hi > lo + 1) {
		int mid = (hi + lo) / 2;
		if (users[mid].id <= id) lo = mid;
		else hi = mid;
	}
	return users[lo];
}

void GenerateDataBlock() {
	
	//All ratings generate a data block
	blocks.clear();
	for(int i=0; i<(int)ratings.size(); i++) {
		
		block current;
		const rating & rat = ratings[i];
		const user usr = GetUserById(rat.userid);
		const movie mov = GetMovieById(rat.movieid);
		if (rat.movieid != mov.id) printf("error1!\n");
		if (rat.userid != usr.id) printf("error2!\n");
		
		//Reduce some of the data to simplify tree
		current.data[0] = ReduceMovie(mov.id);
		current.data[1] = ReduceAge(usr.age);
		current.data[2] = ReduceGender(usr.gender);
		current.data[3] = ReduceOccupation(usr.occupation);
		current.data[4] = ReduceZipCode(usr.zipcode);
		current.data[5] = ReduceTimestamp(rat.timestamp);
		current.value = min(5, max(1, rat.value));
		
		//Add to dataset
		blocks.push_back(current);
	}
}

struct node {
	int value;
	int atribute;
	int majorityValue;
	int size;
	map<int, node*> sons;
	node(int _value) : value(_value), atribute(-1), majorityValue(3), size(1) {}
};

inline int GetMajorityValue(pblock begin, pblock end) {
	
	//Iterate over array and get most frequent
	int stars[6], ans = 3;
	memset(&stars, 0, sizeof stars);
	for(pblock it = begin; it != end; it++) {
		stars[it->value]++;
		if (stars[it->value] > stars[ans]) ans = it->value;
	}
	return ans;
}

bool comp(const block & a, const block & b) {
	//Sort an array of block according to data of atribute curAtributeComp
	for(int i=0; i<MAX_ATRIBUTE; i++) {
		if (a.data[i] != b.data[i]) return a.data[i] < b.data[i];
	}
	return false;
}

node* BuildDecisionTree(pblock begin, pblock end, int atribute, int pattern) {
	
	//If there are no examples, return pattern
	if (begin == end) {
		return new node(pattern);
	}
	
	//If all blocks have same value, return that value
	bool allequal = true;
	for(pblock it = begin; (it+1) != end && allequal; it++) {
		if (it->value != (it+1)->value) allequal = false;
	}
	if (allequal) {
		return new node(begin->value);
	}
	
	//If there are no data left to analyse, return majority value
	if (atribute == MAX_ATRIBUTE) {
		return new node(GetMajorityValue(begin, end));
	}
	
	//Recursively build tree node
	node* tree = new node(-1);
	tree->majorityValue = GetMajorityValue(begin, end);
	tree->atribute = atribute;
	
	//Build subtree
	for(pblock it = begin, last = begin; it != end; it++) {
		if ((it+1) == end || (it+1)->data[atribute] != it->data[atribute]) {
			tree->sons[it->data[atribute]] = BuildDecisionTree(last, it+1, atribute+1, tree->majorityValue);
			tree->size += tree->sons[it->data[atribute]]->size;
			last = it+1;
		}
	}
	
	//Done
	return tree;
}

int Query(const block & queryBlock, node* treeNode, int atribute) {
	
	//If this node has a defined value, return it
	if (treeNode->value != -1) {
		return treeNode->value;
	}
	
	//If there is no subnode for atribute, return majority value
	if (atribute == MAX_ATRIBUTE || !treeNode->sons.count(queryBlock.data[atribute])) {
		return treeNode->majorityValue;
	}
	
	//Keep searching
	return Query(queryBlock, treeNode->sons[queryBlock.data[atribute]], atribute+1);
}

void Delete(node* treeNode) {
	if (treeNode == NULL) return;
	
	//Delete sons then delete itself
	for(map<int, node*>::iterator it = treeNode->sons.begin(); it != treeNode->sons.end(); it++) {
		Delete(it->second);
	}
	delete treeNode;
}

int movieAvgRatings[MAX_MOVIES];
double sumRatings[MAX_MOVIES];
int numRatings[MAX_MOVIES];

void ComputeMovieAverageRatings(int trainingSetSize, int nMovies) {
	
	//Compute sums of all ratings and number of ratings for each movie
	for(int i=1; i<=nMovies; i++) {
		sumRatings[i] = 0.0;
		numRatings[i] = 0;
	}
	for(int it = 0; it < trainingSetSize; it++) {
		int movieid = blocks[it].data[0];
		numRatings[movieid]++;
		sumRatings[movieid] += (double)blocks[it].value;
	}

	//Compute average and aproximate
	for(int i=1; i<=nMovies; i++) {
		if (numRatings[i] == 0) movieAvgRatings[i] = 3;
		else movieAvgRatings[i] = int(sumRatings[i]/numRatings[i] + 0.4999999);
		if (movieAvgRatings[i] < 1.0 || movieAvgRatings[i] > 5.0) printf("%.3f %d %.3f\n", sumRatings[i], numRatings[i], movieAvgRatings[i]);
	}
}

double decisionTreeStd;
double avgStd;
double decisionTreeDiff;
double avgDiff;
node* decisionTree;

void RunDataset(pblock begin, pblock end) {
	
	//Initialize
	int datasetSize = int(end - begin);
	decisionTreeStd = 0.0;
	avgStd = 0.0;
	decisionTreeDiff = 0.0;
	avgDiff = 0.0;
	
	//iterate over blocks
	for(pblock it = begin; it != end; it++) {
		
		//Compute values
		int decisionTreeRating = Query(*it, decisionTree, 0);
		int averageRating = movieAvgRatings[it->data[0]];
		int trueRating = it->value;
		decisionTreeStd += (decisionTreeRating - trueRating)*(decisionTreeRating - trueRating);
		avgStd += (averageRating - trueRating)*(averageRating - trueRating);
		decisionTreeDiff += (decisionTreeRating == trueRating ? 1.0 : 0.0);
		avgDiff += (averageRating == trueRating ? 1.0 : 0.0);
	}
	
	//Standart deviation and sum of absolute values of diferences
	decisionTreeStd = sqrt(decisionTreeStd / datasetSize);
	avgStd = sqrt(avgStd / datasetSize);
	decisionTreeDiff /= datasetSize;
	avgDiff /= datasetSize;
}

int main() {
	
	//Read input
	ReadMovies();
	ReadRatings();
	ReadUsers();
	
	//Build decision tree data from ratings, users and movies info
	printf("Generating data blocks... ");
	GenerateDataBlock();
	printf("done, number os blocks = %u\n", blocks.size());
	
	//Try multiple ratios for validation set size
	//srand(time(NULL));
	for (int mult = 1; mult <= 10; mult++) {
		//printf("%d\\%% ", mult*5);
		
		//Randomly separate VALIDATIONSET_RATIO*mult*100% of data for validation set
		printf("Separating %.2f%% of data blocks for validation set... ", VALIDATIONSET_RATIO*mult*100.0);
		int validationSetSize = mult*blocks.size()*VALIDATIONSET_RATIO;
		int trainingSetSize = blocks.size() - validationSetSize;
		random_shuffle(blocks.begin(), blocks.end());
		printf("done, %d blocks\n", validationSetSize);
		
		//Get average ratings for all movies and build decision tree
		printf("Building tree and computing average movie ratings... ");
		ComputeMovieAverageRatings(trainingSetSize, movies.size());
		sort(blocks.begin(), blocks.begin() + trainingSetSize, comp);
		decisionTree = BuildDecisionTree(blocks.begin(), blocks.begin() + trainingSetSize, 0, 3);
		printf("done, tree size %d\n", decisionTree->size);
		
		//Run training set
		printf("Testing training set (size %d)... ", trainingSetSize);
		RunDataset(blocks.begin(), blocks.begin() + trainingSetSize);
		printf("done, DT %.3f-%.2f%%, avg. %.3f-%.2f%%\n", decisionTreeStd, 100.0*decisionTreeDiff, avgStd, 100.0*avgDiff);
		//printf("& %.3f-%.2f\\%% & %.3f-%.2f\\%% ", decisionTreeStd, 100.0*decisionTreeDiff, avgStd, 100.0*avgDiff);
		
		//Run validation set
		printf("Testing validation set (size %d)... ", validationSetSize);
		RunDataset(blocks.begin() + trainingSetSize, blocks.end());
		printf("done, DT %.3f-%.2f%%, avg. %.3f-%.2f%%\n", decisionTreeStd, 100.0*decisionTreeDiff, avgStd, 100.0*avgDiff);
		//printf("& %.3f-%.2f\\%% & %.3f-%.2f\\%%\\\\\n", decisionTreeStd, 100.0*decisionTreeDiff, avgStd, 100.0*avgDiff);
		
		//Destroy tree at end
		Delete(decisionTree);
	}
	return 0;
}