#include "config.h"

#include "test_queue_buckets.h"

#include "utils/instrumentation.h"
#include "utils/queue_buckets.h"

CPPUNIT_TEST_SUITE_REGISTRATION(TestQueueBuckets);

struct test_constants {
  static const int bucket_count = 2;

  static const torrent::instrumentation_enum instrumentation_added[bucket_count];
  static const torrent::instrumentation_enum instrumentation_removed[bucket_count];
  static const torrent::instrumentation_enum instrumentation_total[bucket_count];

  template <typename Type>
  static void destroy(Type& obj);
};

const torrent::instrumentation_enum test_constants::instrumentation_added[bucket_count] = {
  torrent::INSTRUMENTATION_TRANSFER_REQUESTS_QUEUED_ADDED,
  torrent::INSTRUMENTATION_TRANSFER_REQUESTS_CANCELED_ADDED
};
const torrent::instrumentation_enum test_constants::instrumentation_removed[bucket_count] = {
  torrent::INSTRUMENTATION_TRANSFER_REQUESTS_QUEUED_REMOVED,
  torrent::INSTRUMENTATION_TRANSFER_REQUESTS_CANCELED_REMOVED
};
const torrent::instrumentation_enum test_constants::instrumentation_total[bucket_count] = {
  torrent::INSTRUMENTATION_TRANSFER_REQUESTS_QUEUED_TOTAL,
  torrent::INSTRUMENTATION_TRANSFER_REQUESTS_CANCELED_TOTAL
};

typedef torrent::queue_buckets<int, test_constants> buckets_type;

static int items_destroyed = 0;

template <>
void
test_constants::destroy<int>(__UNUSED int& obj) {
  items_destroyed++;
}

#define FILL_BUCKETS(s_0, s_1)                  \
  for (int i = 0; i < s_0; i++)                 \
    buckets.push_front(0, i);                   \
  for (int i = 0; i < s_1; i++)                 \
    buckets.push_front(1, s_0 + i);

#define VERIFY_QUEUE_SIZES(s_0, s_1)            \
  CPPUNIT_ASSERT(buckets.queue_size(0) == s_0); \
  CPPUNIT_ASSERT(buckets.queue_size(1) == s_1);

#define VERIFY_INSTRUMENTATION(a_0, r_0, t_0, a_1, r_1, t_1)            \
  CPPUNIT_ASSERT(torrent::instrumentation_values[test_constants::instrumentation_added[0]] == a_0); \
  CPPUNIT_ASSERT(torrent::instrumentation_values[test_constants::instrumentation_removed[0]] == r_0); \
  CPPUNIT_ASSERT(torrent::instrumentation_values[test_constants::instrumentation_total[0]] == t_0); \
  CPPUNIT_ASSERT(torrent::instrumentation_values[test_constants::instrumentation_added[1]] == a_1); \
  CPPUNIT_ASSERT(torrent::instrumentation_values[test_constants::instrumentation_removed[1]] == r_1); \
  CPPUNIT_ASSERT(torrent::instrumentation_values[test_constants::instrumentation_total[1]] == t_1);

#define VERIFY_ITEMS_DESTROYED(count)           \
  CPPUNIT_ASSERT(items_destroyed == count);     \
  items_destroyed = 0;

//
// Basic tests:
//

void
TestQueueBuckets::test_basic() {
  torrent::instrumentation_initialize();

  buckets_type buckets;

  VERIFY_QUEUE_SIZES(0, 0);

  buckets.push_front(0, int());
  VERIFY_QUEUE_SIZES(1, 0);
  buckets.push_back(0, int());
  VERIFY_QUEUE_SIZES(2, 0);
  VERIFY_INSTRUMENTATION(2, 0, 2, 0, 0, 0);  

  buckets.push_front(1, int());
  VERIFY_QUEUE_SIZES(2, 1);
  buckets.push_back(1, int());
  VERIFY_QUEUE_SIZES(2, 2);
  VERIFY_INSTRUMENTATION(2, 0, 2, 2, 0, 2);  

  buckets.pop_front(0);
  VERIFY_QUEUE_SIZES(1, 2);
  buckets.pop_back(0);
  VERIFY_QUEUE_SIZES(0, 2);
  VERIFY_INSTRUMENTATION(2, 2, 0, 2, 0, 2);  

  buckets.pop_front(1);
  VERIFY_QUEUE_SIZES(0, 1);
  buckets.pop_back(1);
  VERIFY_QUEUE_SIZES(0, 0);
  VERIFY_INSTRUMENTATION(2, 2, 0, 2, 2, 0);  
}

void
TestQueueBuckets::test_erase() {
  items_destroyed = 0;
  torrent::instrumentation_initialize();

  buckets_type buckets;

  FILL_BUCKETS(10, 5);

  VERIFY_QUEUE_SIZES(10, 5);
  VERIFY_INSTRUMENTATION(10, 0, 10, 5, 0, 5);  
  
  buckets.destroy(0, buckets.begin(0) + 3, buckets.begin(0) + 7);
  VERIFY_ITEMS_DESTROYED(4);
  VERIFY_QUEUE_SIZES(6, 5);
  VERIFY_INSTRUMENTATION(10, 4, 6, 5, 0, 5);  

  buckets.destroy(1, buckets.begin(1) + 0, buckets.begin(1) + 5);
  VERIFY_ITEMS_DESTROYED(5);
  VERIFY_QUEUE_SIZES(6, 0);
  VERIFY_INSTRUMENTATION(10, 4, 6, 5, 5, 0);
}

void
TestQueueBuckets::test_destroy_range() {
  items_destroyed = 0;
  torrent::instrumentation_initialize();

  buckets_type buckets;

  FILL_BUCKETS(10, 5);

  VERIFY_QUEUE_SIZES(10, 5);
  VERIFY_INSTRUMENTATION(10, 0, 10, 5, 0, 5);  
  
  buckets.destroy(0, buckets.begin(0) + 3, buckets.begin(0) + 7);
  VERIFY_ITEMS_DESTROYED(4);
  VERIFY_QUEUE_SIZES(6, 5);
  VERIFY_INSTRUMENTATION(10, 4, 6, 5, 0, 5);  

  buckets.destroy(1, buckets.begin(1) + 0, buckets.begin(1) + 5);
  VERIFY_ITEMS_DESTROYED(5);
  VERIFY_QUEUE_SIZES(6, 0);
  VERIFY_INSTRUMENTATION(10, 4, 6, 5, 5, 0);
}

void
TestQueueBuckets::test_move_range() {
  items_destroyed = 0;
  torrent::instrumentation_initialize();

  buckets_type buckets;

  FILL_BUCKETS(10, 5);
  torrent::instrumentation_reset();

  buckets.move_to(0, buckets.begin(0) + 3, buckets.begin(0) + 7, 1);
  VERIFY_ITEMS_DESTROYED(0);

  VERIFY_QUEUE_SIZES(6, 9);
  VERIFY_INSTRUMENTATION(0, 4, 6, 4, 0, 9);  
}
