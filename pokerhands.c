#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void printCards(int const *cards, int n)
{
  int const *c = cards;
  for (int i=0; i<n; i++, c++) {
    int suit = (*c) % 4;
    int rank = (*c) / 4;
    char const *suits = "CDHS";
    char const *ranks = "23456789TJQKA";
    printf("%c%c ", ranks[rank], suits[suit]);
  }
  printf("\n");
}

int cmp(void const *e1, void const *e2)
{
  int f = *((int*)e1), s = *((int*)e2);
  return (f < s) - (f > s);
}

// Assumes cards are unique (i.e. derived from a single deck)
// Does not perform suit-ordering
int const *calculateHand(int const *cards, int n)
{
  static int hand[6];
  for (int i=0; i<6; i++) hand[i] = 0;
  if (n == 0) {
    hand[0] = -1;
    return hand;
  }
  // Iterate
  int cntRank[14], cntSuit[4], suits[14];
  for (int i=0; i<14; i++) {
    cntRank[i] = 0;
    suits[i] = 0;
  }
  for (int i=0; i<4; i++) cntSuit[i] = 0;
  int maxRank = 0, maxSuit = 0;
  for (int i=0; i<n; i++) {
    int c = cards[i];
    int s = c % 4;
    int r = (c / 4) + 1;
    cntRank[r]++;
    cntSuit[s]++;
    if (cntRank[r] > maxRank) maxRank = cntRank[r];
    if (cntSuit[s] > maxSuit) maxSuit = cntSuit[s];
    suits[r] = suits[r] | (1 << s);
    if (r == 13) {
      cntRank[0]++;
      if (cntRank[0] > maxRank) maxRank = cntRank[0];
      suits[0] = suits[0] | (1 << s);
    }
  }
  // Straights
  int bestStraight = 0, straightCnt=0;
  for (int i=13; i>=0; i--) {
    if (cntRank[i] > 0)
      straightCnt++;
    else
      straightCnt = 0;
    if (straightCnt >= 5) {
      if (bestStraight == 0) bestStraight = i + 4;
      if (suits[i] & suits[i + 1] & suits[i + 2] & suits[i + 3] & suits[i + 4]) {
        // Straight flush
        if (i + 4 == 13) {
          // Royal flush
          hand[0] = 9;
          return hand;
        }
        hand[0] = 8;
        hand[1] = i + 4;
        return hand;
      }
    }
  }
  // Order the cards by frequency
  for (int i=0; i<5; i++) {
    int max = 0, r = 0;
    for (int j=13; j>0; j--) {
      int valid = 1;
      for (int k=0; k<i; k++) {
        if (hand[k + 1] == j) {
          valid = 0;
          break;
        }
      }
      if (!valid) continue;
      if (cntRank[j] > max) {
        max = cntRank[j];
        r = j;
      }
    }
    if (max == 0) break;
    hand[i + 1] = r;
  }
  cntRank[0] = 0;
  // 4 of a kind
  if (maxRank == 4) {
    hand[0] = 7;
    for (int i=3; i<6; i++) hand[i] = 0;
    return hand;
  }
  // Full house
  if (maxRank == 3 && cntRank[hand[2]] >= 2) {
    hand[0] = 6;
    for (int i=3; i<6; i++) hand[i] = 0;
    return hand;
  }
  // Flush
  if (maxSuit >= 5) {
    hand[0] = 5;
    int gsuits = 0;
    for (int i=0; i<4; i++) {
      if (cntSuit[i] >= 5) gsuits = gsuits | (1 << i);
    }
    int r = 13;
    for (int i=1; i<6; i++) {
      while (!(gsuits & suits[r])) r--;
      hand[i] = r;
      gsuits = gsuits & suits[r];
      r--;
    }
    return hand;
  }
  // Straight
  if (bestStraight > 0) {
    hand[0] = 4;
    hand[1] = bestStraight;
    for (int i=2; i<6; i++) hand[i] = 0;
    return hand;
  }
  // 3 of a kind
  if (maxRank == 3) {
    hand[0] = 3;
    for (int i=4; i<6; i++) hand[i] = 0;
    return hand;
  }
  // 2 pair
  if (maxRank == 2 && cntRank[hand[2]] == 2) {
    hand[0] = 2;
    for (int i=4; i<6; i++) hand[i] = 0;
    return hand;
  }
  // 1 pair, high card
  hand[0] = maxRank - 1;
  if (maxRank == 2) hand[5] = 0;
  return hand;
}

char const *getRankName(int rank, int pl)
{
  char const *ranks[] = {
    "two", "three", "four", "five", "six", "seven",
    "eight", "nine", "ten", "jack", "queen", "king",
    "ace"
  };
  char const *ranksp[] = {
    "twos", "threes", "fours", "fives", "sixes", "sevens",
    "eights", "nines", "tens", "jacks", "queens", "kings",
    "aces"
  };
  return (pl ? ranksp[rank] : ranks[rank]);
}

void printHand(int const *hand)
{
  if (hand[0] == 9) {
    printf("royal flush\n");
    return;
  }
  if (hand[0] == 8) {
    printf("%s-high straight flush\n", getRankName(hand[1] - 1, 0));
    return;
  }
  if (hand[0] == 7) {
    printf("four %s\n", getRankName(hand[1] - 1, 1));
    if (hand[2]) printf("%s kicker\n", getRankName(hand[2] - 1, 0));
    return;
  }
  if (hand[0] == 6) {
    printf("full house (%s and %s)\n", getRankName(hand[1] - 1, 1), getRankName(hand[2] - 1, 1));
    return;
  }
  if (hand[0] == 5) {
    printf("flush\n");
    if (hand[1]) printf("%s kicker\n", getRankName(hand[1] - 1, 0));
    if (hand[2]) printf("%s kicker\n", getRankName(hand[2] - 1, 0));
    if (hand[3]) printf("%s kicker\n", getRankName(hand[3] - 1, 0));
    if (hand[4]) printf("%s kicker\n", getRankName(hand[4] - 1, 0));
    if (hand[5]) printf("%s kicker\n", getRankName(hand[5] - 1, 0));
    return;
  }
  if (hand[0] == 4) {
    printf("%s-high straight\n", getRankName(hand[1] - 1, 0));
    return;
  }
  if (hand[0] == 3) {
    printf("three of a kind (%s)\n", getRankName(hand[1] - 1, 0));
    if (hand[2]) printf("%s kicker\n", getRankName(hand[2] - 1, 0));
    if (hand[3]) printf("%s kicker\n", getRankName(hand[3] - 1, 0));
    return;
  }
  if (hand[0] == 2) {
    printf("two pairs (%s and %s)\n", getRankName(hand[1] - 1, 1), getRankName(hand[2] - 1, 1));
    if (hand[3]) printf("%s kicker\n", getRankName(hand[3] - 1, 0));
    return;
  }
  if (hand[0] == 1) {
    printf("one pair (%s)\n", getRankName(hand[1] - 1, 0));
    if (hand[2]) printf("%s kicker\n", getRankName(hand[2] - 1, 0));
    if (hand[3]) printf("%s kicker\n", getRankName(hand[3] - 1, 0));
    if (hand[4]) printf("%s kicker\n", getRankName(hand[4] - 1, 0));
    return;
  }
  if (hand[0] == 0) {
    printf("%s-high\n", getRankName(hand[1] - 1, 0));
    if (hand[2]) printf("%s kicker\n", getRankName(hand[2] - 1, 0));
    if (hand[3]) printf("%s kicker\n", getRankName(hand[3] - 1, 0));
    if (hand[4]) printf("%s kicker\n", getRankName(hand[4] - 1, 0));
    if (hand[5]) printf("%s kicker\n", getRankName(hand[5] - 1, 0));
    return;
  }
  printf("literally nothing\n");
}

int main()
{
  int cards[52];
  int n = 0;
  char r, s, ro, so, e = 0;
  char const *rs = "23456789tjqka", *ss = "cdhs";

  while (!e) {
    scanf("%c%c", &r, &s);
    ro = 0;
    so = 0;
    while (ro < 13 && rs[ro] != r) ro++;
    if (ro == 13) {
      e = 1;
      break;
    }
    while (so < 4 && ss[so] != s) so++;
    if (so == 4) {
      e = 1;
      break;
    }
    cards[n] = (ro * 4) + so;
    n++;
  }

  printCards(cards, n);

  int const *h = calculateHand(cards, n);
  printf("%i %i %i %i %i %i\n", h[0], h[1], h[2], h[3], h[4], h[5]);

  printHand(calculateHand(cards, n));

  return 0;
}

int main2()
{
  int cards[5];
  int hands[10];

  for (int i=0; i<10; i++) hands[i] = 0;

  for (cards[0]=0; cards[0]<48; cards[0]++) {
    for (cards[1]=cards[0]+1; cards[1]<49; cards[1]++) {
      for (cards[2]=cards[1]+1; cards[2]<50; cards[2]++) {
        for (cards[3]=cards[2]+1; cards[3]<51; cards[3]++) {
          for (cards[4]=cards[3]+1; cards[4]<52; cards[4]++) {
            int const *h = calculateHand(cards, 5);
            hands[h[0]]++;
          }
        }
      }
    }
  }

  for (int i=0; i<10; i++) {
    printf("%i: %i\n", i, hands[i]);
  }

  return 0;
}
