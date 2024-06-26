/* Automatically generated nanopb constant definitions */
/* Generated by nanopb-0.4.8 */

#include "simple.pb.h"
#if PB_PROTO_HEADER_VERSION != 40
#error Regenerate this file with the current version of nanopb generator.
#endif


/* The following messages exceed 64kB in size: QueryResult, Query */

/* The PB_FIELD_32BIT compilation option must be defined to support messages that exceed 64 kB in size. */
#ifndef PB_FIELD_32BIT
#error Enable PB_FIELD_32BIT to support messages exceeding 64kB in size: QueryResult, Query
#endif
PB_BIND(Element, Element, 2)


PB_BIND(Condition, Condition, AUTO)


PB_BIND(Document, Document, 4)


PB_BIND(QueryResult, QueryResult, 4)


PB_BIND(Query, Query, 4)






#ifndef PB_CONVERT_DOUBLE_FLOAT
/* On some platforms (such as AVR), double is really float.
 * To be able to encode/decode double on these platforms, you need.
 * to define PB_CONVERT_DOUBLE_FLOAT in pb.h or compiler command line.
 */
PB_STATIC_ASSERT(sizeof(double) == 8, DOUBLE_MUST_BE_8_BYTES)
#endif

