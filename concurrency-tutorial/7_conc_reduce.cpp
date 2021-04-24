#include <concore/conc_reduce.hpp>

#include "../common/utils.hpp"

#include <ctype.h>

struct simple_histogram {
    static constexpr int num_letters = 'z' - 'a';

    int cnt_[num_letters];

    simple_histogram() {
        for (int i = 0; i < num_letters; i++)
            cnt_[i] = 0;
    }

    simple_histogram(const simple_histogram& other) {
        for (int i = 0; i < num_letters; i++)
            cnt_[i] = other.cnt_[i];
    }

    simple_histogram& operator=(const simple_histogram& other) {
        for (int i = 0; i < num_letters; i++)
            cnt_[i] = other.cnt_[i];
        return *this;
    }

    void print() const {
        int sum = std::accumulate(std::begin(cnt_), std::end(cnt_), 0);
        for (int i = 0; i < num_letters; i++) {
            printf("  %c   ", char('a' + i));
        }
        printf("\n");
        for (int i = 0; i < num_letters; i++) {
            printf("%4.1f%% ", double(cnt_[i] * 100.0) / double(sum));
        }
        printf("\n");
    }

    void add_text_part(std::string_view str) {
        for (auto c : str) {
            c = tolower(c);
            if (isalpha(c))
                cnt_[c - 'a']++;
        }
    }

    void join_with(const simple_histogram& other) {
        for (int i = 0; i < num_letters; i++)
            cnt_[i] += other.cnt_[i];
    }
};

using text = std::vector<std::string>;

simple_histogram compute_histogram(const text& t) {
    CONCORE_PROFILING_FUNCTION();
    simple_histogram res;

    auto op = [](simple_histogram lhs, const std::string& s) {
        CONCORE_PROFILING_SCOPE_N("op");
        lhs.add_text_part(s);
        sleep_for(2ms); // artificially make this longer
        return lhs;
    };
    auto reduction = [](simple_histogram lhs, const simple_histogram& rhs) {
        CONCORE_PROFILING_SCOPE_N("reduction");
        lhs.join_with(rhs);
        sleep_for(1ms); // artificially make this longer
        return lhs;
    };
    return concore::conc_reduce(t.begin(), t.end(), simple_histogram{}, op, reduction);
}

int main() {
    profiling_sleep profiling_helper;
    CONCORE_PROFILING_FUNCTION();

    std::vector<std::string> text1 = {"Happy", "families", "are", "all", "alike;", "every",
            "unhappy", "family", "is", "unhappy", "in", "its", "own", "way.", "Everything", "was",
            "in", "confusion", "in", "the", "Oblonskys’", "house.", "The", "wife", "had",
            "discovered", "that", "the", "husband", "was", "carrying", "on", "an", "intrigue",
            "with", "a", "French", "girl,", "who", "had", "been", "a", "governess", "in", "their",
            "family,", "and", "she", "had", "announced", "to", "her", "husband", "that", "she",
            "could", "not", "go", "on", "living", "in", "the", "same", "house", "with", "him.",
            "This", "position", "of", "affairs", "had", "now", "lasted", "three", "days,", "and",
            "not", "only", "the", "husband", "and", "wife", "themselves,", "but", "all", "the",
            "members", "of", "their", "family", "and", "household,", "were", "painfully",
            "conscious", "of", "it.", "Every", "person", "in", "the", "house", "felt", "that",
            "there", "was", "no", "sense", "in", "their", "living", "together,", "and", "that",
            "the", "stray", "people", "brought", "together", "by", "chance", "in", "any", "inn",
            "had", "more", "in", "common", "with", "one", "another", "than", "they,", "the",
            "members", "of", "the", "family", "and", "household", "of", "the", "Oblonskys.", "The",
            "wife", "did", "not", "leave", "her", "own", "room,", "the", "husband", "had", "not",
            "been", "at", "home", "for", "three", "days.", "The", "children", "ran", "wild", "all",
            "over", "the", "house;", "the", "English", "governess", "quarreled", "with", "the",
            "housekeeper,", "and", "wrote", "to", "a", "friend", "asking", "her", "to", "look",
            "out", "for", "a", "new", "situation", "for", "her;", "the", "man", "-", "cook", "had",
            "walked", "off", "the", "day", "before", "just", "at", "dinner", "time;", "the",
            "kitchen", "-", "maid,", "and", "the", "coachman", "had", "given", "warning."};

    std::vector<std::string> text2 = {"DURING", "the", "whole", "of", "a", "dull,", "dark,", "and",
            "soundless", "day", "in", "the", "autumn", "of", "the", "year,", "when", "the",
            "clouds", "hung", "oppressively", "low", "in", "the", "heavens,", "I", "had", "been",
            "passing", "alone,", "on", "horseback,", "through", "a", "singularly", "dreary",
            "tract", "of", "country,", "and", "at", "length", "found", "myself,", "as", "the",
            "shades", "of", "the", "evening", "drew", "on,", "within", "view", "of", "the",
            "melancholy", "House", "of", "Usher.", "I", "know", "not", "how", "it", "was—but,",
            "with", "the", "first", "glimpse", "of", "the", "building,", "a", "sense", "of",
            "insufferable", "gloom", "pervaded", "my", "spirit.", "I", "say", "insufferable;",
            "for", "the", "feeling", "was", "unrelieved", "by", "any", "of", "that",
            "half-pleasurable,", "because", "poetic,", "sentiment,", "with", "which", "the", "mind",
            "usually", "receives", "even", "the", "sternest", "natural", "images", "of", "the",
            "desolate", "or", "terrible.", "I", "looked", "upon", "the", "scene", "before",
            "me—upon", "the", "mere", "house,", "and", "the", "simple", "landscape", "features",
            "of", "the", "domain—upon", "the", "bleak", "walls—upon", "the", "vacant", "eye-like",
            "windows—upon", "a", "few", "rank", "sedges—and", "upon", "a", "few", "white", "trunks",
            "of", "decayed", "trees—with", "an", "utter", "depression", "of", "soul", "which", "I",
            "can", "compare", "to", "no", "earthly", "sensation", "more", "properly", "than", "to",
            "the", "after-dream", "of", "the", "reveller", "upon", "opium—the", "bitter", "lapse",
            "into", "every-day", "life—the", "hideous", "dropping", "off", "of", "the", "veil.",
            "There", "was", "an", "iciness,", "a", "sinking,", "a", "sickening", "of", "the",
            "heart—an", "unredeemed", "dreariness", "of", "thought", "which", "no", "goading", "of",
            "the", "imagination", "could", "torture", "into", "aught", "of", "the", "sublime.",
            "What", "was", "it—I", "paused", "to", "think—what", "was", "it", "that", "so",
            "unnerved", "me", "in", "the", "contemplation", "of", "the", "House", "of", "Usher?",
            "It", "was", "a", "mystery", "all", "insoluble;", "nor", "could", "I", "grapple",
            "with", "the", "shadowy", "fancies", "that", "crowded", "upon", "me", "as", "I",
            "pondered.", "I", "was", "forced", "to", "fall", "back", "upon", "the",
            "unsatisfactory", "conclusion,", "that", "while,", "beyond", "doubt,", "there", "are",
            "combinations", "of", "very", "simple", "natural", "objects", "which", "have", "the",
            "power", "of", "thus", "affecting", "us,", "still", "the", "analysis", "of", "this",
            "power", "lies", "among", "considerations", "beyond", "our", "depth.", "It", "was",
            "possible,", "I", "reflected,", "that", "a", "mere", "different", "arrangement", "of",
            "the", "particulars", "of", "the", "scene,", "of", "the", "details", "of", "the",
            "picture,", "would", "be", "sufficient", "to", "modify,", "or", "perhaps", "to",
            "annihilate", "its", "capacity", "for", "sorrowful", "impression;", "and,", "acting",
            "upon", "this", "idea,", "I", "reined", "my", "horse", "to", "the", "precipitous",
            "brink", "of", "a", "black", "and", "lurid", "tarn", "that", "lay", "in", "unruffled",
            "lustre", "by", "the", "dwelling,", "and", "gazed", "down—but", "with", "a", "shudder",
            "even", "more", "thrilling", "than", "before—upon", "the", "remodelled", "and",
            "inverted", "images", "of", "the", "gray", "sedge,", "and", "the", "ghastly",
            "tree-stems,", "and", "the", "vacant", "and", "eye-like", "windows."};

    auto h1 = compute_histogram(text1);
    auto h2 = compute_histogram(text2);
    h1.print();
    h2.print();

    return 0;
}
