#ifndef __LIB_LINEAR_H__
#define __LIB_LINEAR_H__

#include "../liblinear-multicore-2.11-2/linear.h"
#include "../utils/utils.h"
#include "../utils/parameters.h"
#define Malloc(type,n) (type *)malloc((n)*sizeof(type))

namespace LiblinearRelated
{

struct feature_node *x_space;
struct parameter param;
struct problem prob;
struct model* model_;

struct feature_node *xx_space;

inline void train(const vector< vector<double> > &_features, const vector<double> _results) {
	assert(_features.size() == _results.size());
	void (*print_func)(const char*) = NULL;
	
	set_print_string_function(print_func);

	param.solver_type = L2R_LR;
	param.C = 1;
	param.p = 0.1;
	param.eps = 0.01;
	prob.bias = 1;

	param.nr_weight = 0;
	param.nr_thread = NTHREADS;

	if (prob.bias >= 0) {
		prob.n = _features.back().size() + 1;
	}
	else {
		prob.n = _features.back().size();
	}

	prob.l = _features.size();
	prob.y = Malloc(double, _features.size());
	prob.x = Malloc(struct feature_node *, _features.size());
	x_space = Malloc(struct feature_node, _features.size() * (_features.back().size() + 2));
	xx_space = Malloc(struct feature_node, _features.back().size() + 2);

	size_t j = 0;
	for(int i = 0; i < _features.size(); ++i) {
		prob.x[i] = &x_space[j];
		prob.y[i] = _results[i];
		for (int k = 0; k < _features[i].size(); ++k) {
			x_space[j].index = k + 1;
			x_space[j].value = _features[i][k];
			++j;
		}
		if (prob.bias >= 0) {
			x_space[j].index = prob.n;
			x_space[j++].value = prob.bias;

		}
		x_space[j++].index = -1;
	}

	model_=train(&prob, &param);
	
}

inline double predict(const vector<double> &_feature) {
	int i;
	for (i = 0; i < _feature.size(); ++i) {
		xx_space[i].index = i + 1;
		xx_space[i].value = _feature[i];
	}

	if (prob.bias >= 0) {
		xx_space[i].index = prob.n;
		xx_space[i++].value = prob.bias;
	}

	xx_space[i].index = -1;
	assert(i == _feature.size() + 1);
	double prob_estimates[2] = {0.0, 0.0};
	predict_probability(model_, &xx_space[0], prob_estimates);
	//return max(prob_estimates[0], prob_estimates[1]);
	return prob_estimates[1];
}

};

#endif