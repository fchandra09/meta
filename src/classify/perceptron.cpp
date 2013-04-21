#include "classify/perceptron.h"

namespace meta {
namespace classify {

perceptron::perceptron( double alpha, double gamma, double bias,
                        size_t max_iter ) 
        : alpha_{ alpha }, gamma_{ gamma }, bias_{ bias }, 
          max_iter_{ max_iter } { }


double perceptron::get_weight( const class_label & label, 
                               const term_id & term ) const {
    auto weight_it = weights_.find( label );
    if( weight_it == weights_.end() )
        return 0;
    auto term_it = weight_it->second.find( term );
    if( term_it == weight_it->second.end() )
        return 0;
    return term_it->second;
}

void perceptron::zero_weights( const std::vector<index::Document> & docs ) {
    for( const auto & d : docs )
        weights_[ d.getCategory() ] = {};
}

void perceptron::train( const std::vector<index::Document> & docs ) {
    zero_weights( docs );
    for( size_t iter = 0; iter < max_iter_; ++iter ) {
        common::show_progress( iter, max_iter_, 10, "[perceptron]: training " );
        double error_count = 0;
        for( const index::Document & doc : docs ) {
            class_label guess = classify( doc );
            class_label actual = doc.getCategory();
            if( guess != actual ) {
                error_count += 1;
                for( const auto & count : doc.getFrequencies() ) {
                    weights_[ guess ][ count.first ] -= alpha_ * count.second;
                    weights_[ actual ][ count.first ] += alpha_ * count.second;
                }
            }
        }
        if( error_count / docs.size() < gamma_ )
            break;
    }
    common::end_progress("[perceptron]: training ");
}

class_label perceptron::classify( const index::Document & doc ) {
    class_label best_label = weights_.begin()->first;
    double best_dot = 0;
    for( const auto & w : weights_ ) {
        double dot = bias_;
        for( const auto & count : doc.getFrequencies() ) {
            dot += count.second * get_weight( w.first, count.first );
        }
        if( dot > best_dot ) {
            best_dot = dot;
            best_label = w.first;
        }
    }
    return best_label;
}

void perceptron::reset() {
    weights_ = {};
}


}
}