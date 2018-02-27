#ifndef __PARAMETERS_H__
#define __PARAMETERS_H__

#include "../utils/utils.h"

typedef char PATTERN_LEN_TYPE;
typedef unsigned char POS_ID_TYPE;
typedef int TOTAL_TOKENS_TYPE;
typedef int PATTERN_ID_TYPE;
typedef int TOKEN_ID_TYPE;
typedef unsigned long long ULL;
typedef int INDEX_TYPE; // sentence id
typedef short int POSITION_INDEX_TYPE; // position inside a sentence

const string RM_TRAIN_FILE = "tmp_remine/rm_tokenized_train.txt";
const string RM_TEST_FILE = "tmp_remine/rm_tokenized_test.txt";
const string RM_TRAIN_CAPITAL_FILE = "tmp_remine/rm_case_tokenized_train.txt";
// const string RM_TRAIN_DEPS_FILE = "tmp_remine/rm_deps_train.txt";
const string RM_TRAIN_DEPS_FILE = "tmp_remine/deps_train_type.txt";
const string RM_STOPWORDS_FILE = "tmp_remine/tokenized_stopwords.txt";
const string RM_PUNC_FILE = "tmp_remine/tokenized_punctuations.txt";
const string RM_ALL_FILE = "tmp_remine/tokenized_quality.txt";
const string RM_NEGATIVES_FILE = "tmp_remine/tokenized_negatives.txt";
const string RM_QUALITY_FILE = "tmp_remine/tokenized_quality.txt";
const string RM_POS_TAGS_FILE = "tmp_remine/rm_pos_tags_train.txt";
const string RM_TRAIN_EMS_FILE = "tmp_remine/entity_position.txt";

const string RM_TEXT_TO_SEG_FILE = "tmp_remine/rm_tokenized_train.txt";
const string RM_TEXT_TO_SEG_POS_TAGS_FILE = "tmp_remine/rm_pos_tags_train.txt";
const string RM_TEXT_TO_SEG_DEPS_FILE = "tmp_remine/rm_deps_train.txt";


const string TRAIN_FILE = "tmp_remine/tokenized_train.txt";
const string TEST_FILE = "tmp_remine/tokenized_train.txt";
const string TRAIN_CAPITAL_FILE = "tmp_remine/case_tokenized_train.txt";

const string EXTERNAL_PATTERNS = "tmp_remine/boost_patterns.txt";
// const string TRAIN_DEPS_FILE = "remine_extraction/ver2/nyt_6k_deps_rm.txt";
const string STOPWORDS_FILE = "tmp_remine/tokenized_stopwords.txt";
const string PUNC_FILE = "tmp_remine/tokenized_punctuations.txt";
const string ALL_FILE = "tmp_remine/tokenized_quality.txt";
const string NEGATIVES_FILE = "tmp_remine/tokenized_negatives.txt";
const string QUALITY_FILE = "tmp_remine/tokenized_quality.txt";
const string POS_TAGS_FILE = "tmp_remine/pos_tags_train.txt";
const string TRAIN_DEPS_FILE = "tmp_remine/deps_train.txt";
//Modify next line just for NYT13K dataset

const string TEXT_TO_SEG_FILE = "tmp_remine/tokenized_train.txt";
const string TEXT_TO_SEG_POS_TAGS_FILE = "tmp_remine/pos_tags_train.txt";
const string TEXT_TO_SEG_DEPS_FILE = "tmp_remine/deps_train.txt";
// const string TEXT_TO_SEG_DEPS_FILE = "remine_extraction/ver2/nyt_6k_deps_rm.txt";



const TOKEN_ID_TYPE BREAK = -911;

int ITERATIONS = 2;
int MIN_SUP = 30;
int MAX_LEN = 5;
int MAX_POSITIVE = 100;
int NEGATIVE_RATIO = 2;
int NTHREADS = 4;
int POSTAG_SCORE = 0;
bool RELATION_MODE = true;
double SEGMENT_MULTI_WORD_QUALITY_THRESHOLD = 0.50;
double SEGMENT_MULTI_PHRASE_QUALITY_THRESHOLD = 0.45;
double SEGMENT_SINGLE_WORD_QUALITY_THRESHOLD = 0.50;
bool ENABLE_POS_TAGGING = true;
bool ENABLE_POS_PRUNE = false;
string NO_EXPANSION_POS_FILENAME = "";
double DISCARD = 0.05;
string LABEL_FILE = "";
bool INTERMEDIATE = true;
bool ORIGINAL_PUNC = true;
string LABEL_METHOD = "DPDN";
string SEGMENTATION_MODEL = "";
int SEGMENT_QUALITY_TOP_K = 50000;



#endif
