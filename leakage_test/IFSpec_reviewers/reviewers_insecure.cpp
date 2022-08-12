#include <string>
#include <iostream>
#include <vector>
#include <algorithm>
#include <cstdio>
#include <cstdlib>
extern "C" {
  #include <stdint.h>
  #include <unistd.h>
  #include <string.h>
  #include "decode_inputs.h"
}

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

int main(void) {
	ReviewProcess rp = ReviewProcess();

  char *Data = (char *)malloc(1024*1024+1);
  int length = read(STDIN_FILENO, Data, 1024*1024+1);
  if (length == -1 || length == 1024*1024+1) {
    printf("Error! too long\n");
    exit(1);
  }
  
  uint8_t *public_in, *secret_in;
  uint32_t public_len, secret_len;
  find_public_and_secret_inputs(Data, length, &public_in, &public_len, &secret_in, &secret_len);

  int maxSecrets = secret_len % 4 == 0 ? secret_len / 4 : (secret_len / 4) + 1;

  int secretPos = 0, publicPos = 0;
  for (int i = 0; i < maxSecrets; i++) {
    secretPos = 4 * i;

    int reviewerID = 0;
    int secLen = secret_len - secretPos;
 //   printf("secret_len: %d, maxSecrets: %d, secLen: %d\n", secret_len, maxSecrets, secLen);
    for (int j = secretPos; j < secretPos + (secLen < 4 ? secLen : 4); j++) {
        reviewerID |= secret_in[j] << 8 * (j - secretPos);
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
}
