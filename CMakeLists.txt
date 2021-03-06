cmake_minimum_required(VERSION 2.8.4)
project(thesis)

set(ENCODE_FILES encoder.c vec.c allocation.c)
set(DECODE_FILES decoder.c vec.c allocation.c)
set(GAMMA_ENCODE_FILES ${ENCODE_FILES} gamma.c)
set(DELTA_ENCODE_FILES ${ENCODE_FILES} delta.c)
set(GAMMA_DECODE_FILES ${DECODE_FILES} gamma.c)
set(DELTA_DECODE_FILES ${DECODE_FILES} delta.c)
set(BASIC_HASH_FILES hashtable.c allocation.c)
set(LINKED_UNIFORM_FILES ${BASIC_HASH_FILES} linked_list_bucket.c uniform_distribution_hash.c)
set(BST_UNIFORM_FILES ${BASIC_HASH_FILES} binary_search_tree_bucket.c uniform_distribution_hash.c)
set(LINKED_LOG_FILES ${BASIC_HASH_FILES} linked_list_bucket.c logarithmic_distribution_hash.c)
set(BST_LOG_FILES ${BASIC_HASH_FILES} binary_search_tree_bucket.c logarithmic_distribution_hash.c)
set(LINKED_CALCULATED_FILES ${BASIC_HASH_FILES} linked_list_bucket.c calculated_distribution_hash.c)
set(BST_CALCULATED_FILES ${BASIC_HASH_FILES} binary_search_tree_bucket.c calculated_distribution_hash.c)
set(LINKED_UNIFORM_TEST_FILES hash_table_test.c filereader.c ${LINKED_UNIFORM_FILES})
set(BST_UNIFORM_TEST_FILES hash_table_test.c filereader.c ${BST_UNIFORM_FILES})
set(LINKED_LOG_TEST_FILES hash_table_test.c filereader.c ${LINKED_LOG_FILES})
set(LINKED_CALCULATED_TEST_FILES hash_table_test.c filereader.c ${LINKED_CALCULATED_FILES})
set(LINKED_LOG_UNIT_TEST_FILES hashtabletest.c ${LINKED_LOG_FILES})
set(UKKONEN_FILES suffixtree.c ukkonen.c allocation.c filereader.c ${LINKED_UNIFORM_FILES})
set(UNIFORM_TREE_FILES tree_test.c ukkonen.c allocation.c filereader.c ${LINKED_UNIFORM_FILES})
set(BST_UNIFORM_TREE_FILES tree_test.c ukkonen.c allocation.c filereader.c ${BST_UNIFORM_FILES})
set(LOG_TREE_FILES tree_test.c ukkonen.c allocation.c filereader.c ${LINKED_LOG_FILES})
set(BST_LOG_TREE_FILES tree_test.c ukkonen.c allocation.c filereader.c ${BST_LOG_FILES})
set(CALCULATED_TREE_FILES tree_test.c ukkonen.c allocation.c filereader.c ${LINKED_CALCULATED_FILES})
set(BST_CALCULATED_TREE_FILES tree_test.c ukkonen.c allocation.c filereader.c ${BST_CALCULATED_FILES})
add_executable(gamma-encode ${GAMMA_ENCODE_FILES})
target_link_libraries(gamma-encode m)
add_executable(delta-encode ${DELTA_ENCODE_FILES})
target_link_libraries(delta-encode m)
add_executable(gamma-decode ${GAMMA_DECODE_FILES})
target_link_libraries(gamma-decode m)
add_executable(delta-decode ${DELTA_DECODE_FILES})
target_link_libraries(delta-decode m)
add_executable(linked-uniform-test ${LINKED_UNIFORM_TEST_FILES})
add_executable(bst-uniform-test ${BST_UNIFORM_TEST_FILES})
add_executable(linked-log-test ${LINKED_LOG_TEST_FILES})
add_executable(linked-calculated-test ${LINKED_CALCULATED_TEST_FILES})
add_executable(linked-log-unit-test ${LINKED_LOG_UNIT_TEST_FILES})
add_executable(ukkonen ${UKKONEN_FILES})
add_executable(uniform-tree ${UNIFORM_TREE_FILES})
add_executable(bst-uniform-tree ${BST_UNIFORM_TREE_FILES})
add_executable(log-tree ${LOG_TREE_FILES})
add_executable(bst-log-tree ${BST_LOG_TREE_FILES})
add_executable(calculated-tree ${CALCULATED_TREE_FILES})
add_executable(bst-calculated-tree ${BST_CALCULATED_TREE_FILES})