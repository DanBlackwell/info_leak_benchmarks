#include <string>
#include <iostream>
#include <vector>
#include <algorithm>

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

	rp.addReview(42, 1, "Little novelty.");
	rp.addReview(5, 3, "Borderline paper.");
	rp.addReview(7, 4, "Significant contribution.");

	rp.sendNotifications();
}
