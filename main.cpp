//coding exercises following Udacity course on localisation
#include<iostream>
#include<vector>

using std::vector;

// initialize priors assuming vehicle at landmark +/- 1.0 meters position stdev
vector<float> initialize_priors(int map_size, vector<float> landmark_positions, float stdev);

int main() {
    // set standard deviation of position
    float position_stdev = 1.0f;
    // set map horizon distance in meters
    int map_size = 25;
    // initialize landmarks
    vector<float> landmark_positions {5,10,20};
    // initialize priors
    vector<float> priors = initialize_priors(map_size, landmark_positions,
                                          position_stdev);
   // print values to stdout
   for (size_t i = 0; i < priors.size(); i++) {
       std::cout << priors[i] << std::endl;
   }

   return 0;
}

vector<float> initialize_priors(int map_size, vector<float> landmark_positions, float stdev){
    // initialize priors assuming vehicle at landmark +/- 1.0 meters position stdev

    // set all priors to 0.0
    vector<float> priors(map_size, 0.0);
    float norm_term = landmark_positions.size() * (2 * stdev + 1);
    for(size_t i=0; i<landmark_positions.size(); i++){
        for(float j=1; j<=stdev; j++){
            priors.at(int(landmark_positions[i] + j + map_size)%map_size) += 1.0/norm_term;
            priors.at(int(landmark_positions[i] - j + map_size)%map_size) += 1.0/norm_term;
        }
        priors.at(landmark_positions[i]) += 1.0/norm_term;
    }
    return priors;
}
