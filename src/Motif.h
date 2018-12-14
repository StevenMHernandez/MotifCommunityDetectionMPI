#define NUMBER_OF_INTS_PER_MOTIF_STRUCT 3

class Motif {
public:
    int identifier = 0;
    int data[NUMBER_OF_INTS_PER_MOTIF_STRUCT] = { -1, -1, -1 }; // TODO: remove hardcoded values
    int count = 0;
    int capacity = NUMBER_OF_INTS_PER_MOTIF_STRUCT;

    double **L;

    int isLowest = true; // If data[0] is lower than all others (meaning we should do processing on this motif instead of leaving it for another processor

    Motif(double **_L)
    {
        this->L = _L;
    }

    Motif *copy() {
        Motif *m = new Motif(this->L);
        m->identifier = this->identifier;
        m->count = this->count;
        m->capacity = this->capacity;

        for (int i = 0; i < m->count; i++) {
            m->data[i] = this->data[i];
        }

        return m;
    }

    void add(int n) {
        if (this->count > 0) {
            if (this->isLowest && n < this->data[0]) {
                this->isLowest = false;
            }
        }

        this->data[this->count] = n;
        count++;

        if (this->count >= this->capacity) {
            this->determineMotifIdentifier();
        }
    }

    bool contains(int n) {
        for (int i = 0; i < this->count; i++) {
            if (n == this->data[i]) return true;
        }

        return false;
    }

    int determineMotifIdentifier() {
        int _id = 0;

        for (int i = 0; i < this->capacity; i++) {
            if (this->L[this->data[i]][this->data[i + 1 % this->capacity]] > 0) {
                _id += (1 << i);
            }
        }

        this->identifier = _id;

        return _id;
    }
};