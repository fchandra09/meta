wed = {}

def get_dist(a, b):
    x = a.split(' ')
    y = b.split(' ')
    if len(x) == len(y):  #substutition
        for i, j in zip(a, b):
            if (i != j):
                if (i, j) not in wed:
                    wed[(i,j)] = 1
                else:
                    wed[(i, j)] += 1


def compute_wed(path):
    with open(path, 'r') as f:
        for line in f:
           line = line.rstrip().lower()
           parts = line.split(',')
           if len(parts) == 2 and parts[0] != parts[1]:
               get_dist(parts[0], parts[1])
    


if __name__ == '__main__':
    file = 'combinedQueries.txt'
    compute_wed(file)
    print(wed)
    with open('wed.txt', 'a') as f:
        for key, value in wed.iteritems():
            f.write(key[0] +',' + key[1] + ','+ str(value) + '\n')
