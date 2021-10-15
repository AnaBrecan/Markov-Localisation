//coding exercises following Udacity course on localisation
#include<iostream>
#include<vector>
#include<algorithm>
#include "help_functions.h"

using std::vector;
using std::cout;
using std::endl;


// initialize priors assuming vehicle at landmark +/- 1.0 meters position stdev
vector<float> initialize_priors(int map_size, vector<float> landmark_positions,
    float stdev);

// compute the positionprobability using only the modition model
float motion_model(float pseudo_position, float movement, vector<float> priors,
    int map_size, int control_stdev);

//
vector<float> pseudo_range_estimator(vector<float> landmark_positions,
    float pseudo_position);

// observation model: calculate likelihood prob term based on landmark proximity
float observation_model(vector<float> landmark_positions,
                            vector<float> observations, vector<float> pseudo_ranges,
                            float distance_max, float observation_stdev);


int main() {
    // set standard deviation of control
    float control_stdev = 1.0f;

    // set standard deviation of position
    float position_stdev = 1.0f;

    // meters vehicle moves per time step
    float movement_per_timestep = 1.0f;

    // set observation standard deviation
    float observation_stdev = 1.0f;

    // number of x positions on map
    int map_size = 25;

    // set distance max
    float distance_max = map_size;

    // define landmarks
    vector<float> landmark_positions {3, 9, 14, 23};

    // define observations vector, each inner vector represents a set
    //   of observations for a time step
    vector<vector<float> > sensor_obs {{1,7,12,21}, {0,6,11,20}, {5,10,19},
                                       {4,9,18}, {3,8,17}, {2,7,16}, {1,6,15},
                                       {0,5,14}, {4,13}, {3,12}, {2,11}, {1,10},
                                       {0,9}, {8}, {7}, {6}, {5}, {4}, {3}, {2},
                                       {1}, {0}, {}, {}, {}};

    // initialize priors
    vector<float> priors = initialize_priors(map_size, landmark_positions, position_stdev);

    cout << "-----------PRIORS INIT--------------" << endl;
    for (int p = 0; p < priors.size(); ++p){
        cout << priors[p] << endl;
    }
    // initialize posteriors
    vector<float> posteriors(map_size, 0.0);

    // specify time steps
    int time_steps = sensor_obs.size();

    // declare observations vector
    vector<float> observations;

    // cycle through time steps
    for (int t = 0; t < time_steps; ++t) {
        cout << "---------------TIME STEP---------------" << endl;
        cout << "t = " << t << endl;
        cout << "-----Motion----------OBS----------------PRODUCT--" << endl;

        if (!sensor_obs[t].empty()) {
            observations = sensor_obs[t];
        } else {
            observations = {float(distance_max)};
        }

        // step through each pseudo position x (i)
        for (unsigned int i = 0; i < map_size; ++i) {
            float pseudo_position = float(i);

            // get the motion model probability for each x position
            float motion_prob = motion_model(pseudo_position, movement_per_timestep,
                                                                   priors, map_size, control_stdev);
            // get pseudo ranges
            vector<float> pseudo_ranges = pseudo_range_estimator(landmark_positions, pseudo_position);

            // get observation probability
            float observation_prob = observation_model(landmark_positions, observations,
                                                                             pseudo_ranges, distance_max,
                                                                             observation_stdev);

            // calculate the ith posterior
            posteriors[i] = motion_prob * observation_prob;
            cout << motion_prob << "\t" << observation_prob << "\t"  << "\t"  << motion_prob * observation_prob << endl;
        }
        cout << "----------RAW---------------" << endl;
        for (int p = 0; p < posteriors.size(); ++p) {
            cout << posteriors[p] << endl;
        }
        // normalize
        posteriors = Helpers::normalize_vector(posteriors);

        // print to stdout
        cout << posteriors[t] <<  "\t" << priors[t] << endl;
        cout << "----------NORMALIZED---------------" << endl;

        //update
        priors = posteriors;
        for (int p = 0; p < posteriors.size(); ++p) {
            cout << posteriors[p] << endl;
        }

        // print posteriors vectors to stdout
        for (int p = 0; p < posteriors.size(); ++p) {
            cout << posteriors[p] << endl;
        }
    }
return 0;
}

vector<float> initialize_priors(int map_size, vector<float> landmark_positions, float stdev){
    // initialize priors assuming vehicle at landmark +/- 1.0 meters position stdev
    // set all priors to 0.0
    // for each map point priors contains the probability for the car to be at that point
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

// implement the motion model: calculates prob of being at an estimated position at time t
float motion_model(float pseudo_position, float movement, vector<float> priors,
                   int map_size, int control_stdev) {
  // initialize probability
  float position_prob = 0.0f;

  // loop over state space for all possible positions x (convolution):
  for (float j=0; j< map_size; ++j) {
    float next_pseudo_position = j;
    // distance from i to j
    float distance_ij = pseudo_position-next_pseudo_position;

    // transition probabilities:
    float transition_prob = Helpers::normpdf(distance_ij, movement,
                                             control_stdev);
    // estimate probability for the motion model, this is our prior
    position_prob += transition_prob*priors[j];
  }

  return position_prob;
}

vector<float> pseudo_range_estimator(vector<float> landmark_positions, float pseudo_position){
    vector<float> pseudo_ranges;
    for (size_t i=0; i< landmark_positions.size(); i++){
        if (landmark_positions[i] - pseudo_position > 0.0f){
            pseudo_ranges.push_back(landmark_positions[i] - pseudo_position);
        }
    }
    sort(pseudo_ranges.begin(), pseudo_ranges.end());
    return pseudo_ranges;
}

// observation model: calculate likelihood prob term based on landmark proximity
float observation_model(vector<float> landmark_positions,
                        vector<float> observations, vector<float> pseudo_ranges,
                        float distance_max, float observation_stdev) {
  // initialize observation probability
  float distance_prob = 1.0f;

  // run over current observation vector
  for (int z=0; z< observations.size(); ++z) {
    // define min distance
    float pseudo_range_min;

    // check, if distance vector exists
    if (pseudo_ranges.size() > 0) {
      // set min distance
      pseudo_range_min = pseudo_ranges[0];
      // remove this entry from pseudo_ranges-vector
      pseudo_ranges.erase(pseudo_ranges.begin());
    } else {  // no or negative distances: set min distance to a large number
        pseudo_range_min = std::numeric_limits<const float>::infinity();
    }

    // estimate the probability for observation model, this is our likelihood
    distance_prob *= Helpers::normpdf(observations[z], pseudo_range_min,
                                      observation_stdev);
  }

  return distance_prob;
}
