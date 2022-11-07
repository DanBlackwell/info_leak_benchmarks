#include <string>
#include <iostream>
#include <vector>
#include <algorithm>
#include <cstdio>
#include <cstdlib>

#ifdef DFSAN
#  include <sanitizer/dfsan_interface.h>
#endif

extern "C" {
  #include <stdint.h>
  #include <unistd.h>
  #include <string.h>
  #include <assert.h>
#if !defined MSAN && !defined DFSAN
  #include "decode_inputs.h"
#endif
}

#ifdef DFSAN
    dfsan_label secret_label = 1;
#endif

class Review {
public:
	int reviewer_id;
	int score;
	std::string content;
};

int compare(Review &lhs, Review &rhs) {
	if (lhs.reviewer_id != rhs.reviewer_id) {
		return lhs.reviewer_id < rhs.reviewer_id;
	} else if (lhs.score != rhs.score) {
		return lhs.score < rhs.score;
	} else {
		return lhs.content.compare(rhs.content);
	}
}

class ReviewProcess {
public:
	void addReview(int reviewer_id, int score, std::string content) {
		Review r = Review();
		r.reviewer_id = reviewer_id;
		r.score = score;
		r.content = content;
		reviews.push_back(r);
	}

	void sendNotifications() {
		std::sort(reviews.begin(), reviews.end(), compare);
#ifdef DFSAN
    assert(!dfsan_has_label(dfsan_read_label(&reviews, sizeof(reviews)), secret_label));
#endif
		for (int i = 0; i < reviews.size(); i++) {
			Review *r = &reviews[i];
			std::cout << "---" << std::endl;
			std::cout << "Score: " << r->score << std::endl;
			std::cout << "Review: " << r->content << std::endl;
			std::cout << "---" << std::endl;
		}
	}

private:
	std::vector<Review> reviews;
};

#ifdef DFSAN
int main(void) {

    char *Data = (char *)malloc(1024*1024+1);
    int length = read(STDIN_FILENO, Data, 1024*1024+1);
    if (length == -1 || length == 1024*1024+1) {
        printf("Error! too long\n");
        exit(1);
    }
#else
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, uint32_t length) {
#endif

  ReviewProcess rp = ReviewProcess();

  uint8_t *public_in, *secret_in;
  uint32_t public_len, secret_len;
#if defined MSAN || defined DFSAN
    public_in = (uint8_t *)Data;
    public_len = length < 8 ? length : 8;
    secret_in = (uint8_t *)Data + public_len;
    secret_len = length - public_len;
#else
    find_public_and_secret_inputs((const char *)Data, length, &public_in, &public_len, &secret_in, &secret_len);
    if (!public_in || !secret_in) {
      printf("Failed to parse public / secret inputs JSON (expected \'{\"PUBLIC\": \"base64_input\", \"SECRET\": \"base64_input\"}\')\n");
      free(public_in);
      free(secret_in);
      return 1;
    }
#endif

  if (public_len < 2) { 
    printf("public input needs at least 2 bytes\n"); 
#if defined MSAN || defined DFSAN
    // no need to free anything
#else
    free(public_in);
    free(secret_in);
#endif
    return 1; 
  }

#ifdef DFSAN
    dfsan_set_label(secret_label, secret_in, secret_len);
#endif

  int secretPos = 0, publicPos = 0;
  int numReviewers = public_in[0] % 7 + 1;
  for (int i = 0; i < numReviewers; i++) {

    int reviewerID = 0;
    int secLen = secret_len - secretPos;
 //   printf("secret_len: %d, maxSecrets: %d, secLen: %d\n", secret_len, maxSecrets, secLen);
    if (secLen == 0) {
      reviewerID = i;
    } else {
      int len = secLen < 4 ? secLen : 4;

      for (int j = secretPos; j < secretPos + len; j++) {
        reviewerID |= secret_in[j] << 8 * (j - secretPos);
      }

      secretPos += len;
    }

    int reviewScore = 1;
    int pubLen = public_len - publicPos;
    if (publicPos < public_len) {
      reviewScore = public_in[publicPos] % 4 + 1;
      publicPos++;
    }

    char reviewComment[15] = {0};
    int reviewLen = 0;
    if (publicPos < public_len) {
      reviewLen = public_in[publicPos] % 10 + 5;
      publicPos++;
    }

    if (reviewLen > publicPos - public_len) {
      reviewLen = publicPos - public_len;
    }

    memcpy(reviewComment, public_in + publicPos, reviewLen);
    publicPos += reviewLen;

//    printf("Adding review: {id: %d, score: %d, comment: %s}\n", reviewerID, reviewScore, reviewComment);

    rp.addReview(reviewerID, reviewScore, reviewComment);
  }

  rp.sendNotifications();

#if defined MSAN || defined DFSAN
    // no need to free anything
#else
  free(public_in);
  free(secret_in);
#endif

  return 0;
}
