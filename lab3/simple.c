#include <stdio.h>
#include "nanopb/pb_encode.h"
#include "nanopb/pb_decode.h"
#include "simple.pb.h"
#include "nanopb/pb.h"
#include "ast.h"

bool opType_encode(pb_ostream_t *ostream, const pb_field_t *field, void *const *arg) {
    operationType_t *type = (operationType_t *) (*arg);
    return pb_encode_tag_for_field(ostream, field) && pb_encode_varint(ostream, *type);
}

bool opType_decode(pb_istream_t *istream, const pb_field_t *field, void **arg) {
    condition_t *condition = (condition_t *) (*arg);
    operationType_t type;
    bool res = pb_decode_varint(istream, &type);
    condition->opType = (operationType_t) type;
    return res;
}

bool element_encoder(pb_ostream_t *stream, const pb_field_t *field, void *const *arg) {
    element_t *element = (element_t *) (*arg);
    Element proto = Element_init_zero;
    proto.type = element->type;
    strncpy(proto.key, element->key, 13);
    switch (element->type) {
        case INT: {
            proto.value.integerValue = element->integerValue;
            proto.which_value = Element_integerValue_tag;
            break;
        }
        case DOUBLE: {
            proto.value.doubleValue = element->doubleValue;
            proto.which_value = Element_doubleValue_tag;
            break;
        }
        case BOOLEAN: {
            proto.value.boolValue = element->boolValue;
            proto.which_value = Element_boolValue_tag;
            break;
        }
        case STRING: {
            strncpy(proto.value.stringValue, element->stringValue.str, element->stringValue.length);
            proto.which_value = Element_stringValue_tag;
            break;
        }
    }
    return pb_encode_tag_for_field(stream, field) && pb_encode_submessage(stream, Element_fields, &proto);
}

bool element_decoder(pb_istream_t *stream, const pb_field_t *field, void **arg) {
    condition_t *condition = (condition_t *) (*arg);
    Element proto = Element_init_zero;
    bool res = pb_decode(stream, field, &proto);
    elementType_t type = (elementType_t) proto.type;
    char *key = malloc((sizeof(char) * 13));
    strncpy(key, proto.key, 13);
    switch (type) {
        case INT: {
            condition->element = intElement(key, proto.value.integerValue);
            break;
        }
        case DOUBLE: {
            condition->element = doubleElement(key, proto.value.doubleValue);
            break;
        }
        case STRING: {
            size_t size = strlen(proto.value.stringValue);
            char *value = malloc(sizeof(char) * size);
            strcpy(value, proto.value.stringValue);
            condition->element = stringElement(key, value);
            break;
        }
        case BOOLEAN: {
            condition->element = booleanElement(key, proto.value.boolValue);
            break;
        }
    }
    return res;
}

bool condition_encoder(pb_ostream_t *stream, const pb_field_t *field, void *const *arg) {
    condition_t *condition = (condition_t *) (*arg);
    Condition proto = Condition_init_zero;
    proto.opType.arg = &condition->opType;
    proto.opType.funcs.encode = opType_encode;
    if (condition->opType == OP_AND || condition->opType == OP_OR) {
        proto.which_condition = Condition_condition1_tag;
        proto.condition.condition1.arg = condition->condition1;
        proto.condition.condition1.funcs.encode = condition_encoder;
        proto.condition2.arg = condition->condition2;
        proto.condition2.funcs.encode = condition_encoder;
    } else {
        proto.condition.element.arg = condition->element;
        proto.condition.element.funcs.encode = element_encoder;
        proto.which_condition = Condition_element_tag;
    }
    return pb_encode_tag_for_field(stream, field) && pb_encode_submessage(stream, Condition_fields, &proto);
}

bool condition_decoder(pb_istream_t *stream, const pb_field_t *field, void **arg) {
    condition_t *condition = (condition_t *) *arg;
    Condition msg = Condition_init_zero;
    if (condition->opType == OP_AND || condition->opType == OP_OR) {
        if (condition->condition1 == NULL) {
            condition->condition1 = malloc(sizeof(condition_t));
            msg.opType.arg = condition->condition1;
            condition->condition1->condition2 = NULL;
            condition->condition1->condition1 = NULL;
            msg.opType.funcs.decode = opType_decode;
            msg.condition.condition1.arg = condition->condition1;
            msg.condition.condition1.funcs.decode = &condition_decoder;
            msg.condition2.arg = condition->condition1;
            msg.condition2.funcs.decode = &condition_decoder;
        } else if (condition->condition2 == NULL) {
            condition->condition2 = malloc(sizeof(condition_t));
            msg.opType.arg = condition->condition2;
            condition->condition2->condition2 = NULL;
            condition->condition2->condition1 = NULL;
            msg.opType.funcs.decode = opType_decode;
            msg.condition.condition1.arg = condition->condition2;
            msg.condition.condition1.funcs.decode = &condition_decoder;
            msg.condition2.arg = condition->condition2;
            msg.condition2.funcs.decode = &condition_decoder;
        }
        return pb_decode(stream, Condition_fields, &msg);
    } else {
        return element_decoder(stream, Element_fields, &condition);
    }
}


bool condition_decode(pb_istream_t *stream, const pb_field_t *field, void **arg) {
    condition_t *condition = (condition_t *) *arg;
    Condition msg = Condition_init_zero;
    msg.opType.arg = condition;
    msg.opType.funcs.decode = opType_decode;
    msg.condition2.arg = condition;
    msg.condition2.funcs.decode = &condition_decoder;
    msg.condition.condition1.arg = condition;
    msg.condition.condition1.funcs.decode = &condition_decoder;

    return pb_decode(stream, Condition_fields, &msg);
}

int main1() {
    uint8_t buffer[256];
    size_t total_bytes_encoded = 0;
    condition_t *condition = createCondition(OP_AND,
                                             createCondition(OP_OR,
                                                             createCondition(OP_LT, stringElement("hi", "kkkk"), NULL),
                                                             createCondition(OP_NEQ, intElement("mewmew", 17), NULL)),
                                             createCondition(OP_AND,
                                                             createCondition(OP_EQ, stringElement("hui", "meow"), NULL),
                                                             createCondition(OP_NEQ, booleanElement("mewlalala", true),
                                                                             NULL)));

    pb_ostream_t ostream = pb_ostream_from_buffer(buffer, sizeof(buffer));
    if (!condition_encoder(&ostream, Condition_fields, &condition)) {
        const char *error = PB_GET_ERROR(&ostream);
        printf("pb_encode error: %s\n", error);
    } else {
        total_bytes_encoded = ostream.bytes_written;
        printf("Encoded size: %zu\n", total_bytes_encoded);
    }
    condition_t *decodedData = malloc(sizeof(condition_t));
    pb_istream_t istream = pb_istream_from_buffer(buffer, total_bytes_encoded);
    decodedData->condition1 = NULL;
    decodedData->condition2 = NULL;
    decodedData->element = NULL;
    if (!condition_decode(&istream, Condition_fields, &decodedData)) {
        const char *error = PB_GET_ERROR(&istream);
        printf("pb_decode error: %s\n", error);
    } else {
        printf("Bytes decoded: %zu\n", total_bytes_encoded - istream.bytes_left);
        printf("%s\n", decodedData->condition2->condition1->element->stringValue.str);
    }

}

Element fillElement(element_t *element) {
    Element proto = Element_init_zero;
    strncpy(proto.key, element->key, 13);
    switch (element->type) {
        case INT: {
            proto.which_value = Element_integerValue_tag;
            proto.type = ElementType_INT;
            proto.value.integerValue = element->integerValue;
            break;
        }
        case DOUBLE: {
            proto.which_value = Element_doubleValue_tag;
            proto.type = ElementType_DOUBLE;
            proto.value.doubleValue = element->doubleValue;
            break;
        }
        case STRING: {
            proto.which_value = Element_stringValue_tag;
            proto.type = ElementType_STRING;
            strncpy(proto.value.stringValue, element->stringValue.str, element->stringValue.length);
            break;
        }
        case BOOLEAN: {
            proto.which_value = Element_boolValue_tag;
            proto.type = ElementType_BOOLEAN;
            proto.value.boolValue = element->boolValue;
            break;
        }
    }
    return proto;
}

element_t *getElement(Element proto) {
    switch (proto.type) {
        case ElementType_INT: {
            return intElement(proto.key, proto.value.integerValue);
        }
        case ElementType_BOOLEAN: {
            return booleanElement(proto.key, proto.value.boolValue);
        }
        case ElementType_STRING: {
            return stringElement(proto.key, proto.value.stringValue);
        }
        case ElementType_DOUBLE: {
            return doubleElement(proto.key, proto.value.doubleValue);
        }
    }
    return NULL;
}

Document fillDocument(document_t *document) {
    Document proto = Document_init_zero;
    proto.count = document->meta.count;
    strncpy(proto.name, document->meta.name, 13);
    proto.element_count = document->meta.count;
    for (int64_t i = 0; i < document->meta.count; i++) {
        proto.element[i] = fillElement(document->elements[i]);
    }
    return proto;
}

document_t *getDocument(Document proto) {
    document_t *document = createDocument(proto.name);
    for (int64_t i = 0; i < proto.count; i++) {
        addElement(document, getElement(proto.element[i]));
    }
    return document;
}

Condition fillCondition(condition_t *condition) {
    Condition proto = Condition_init_zero;
    proto.opType.arg = &condition->opType;
    proto.opType.funcs.encode = opType_encode;
    if (condition->opType == OP_AND || condition->opType == OP_OR) {
        proto.which_condition = Condition_condition1_tag;
        proto.condition.condition1.arg = condition->condition1;
        proto.condition.condition1.funcs.encode = condition_encoder;
        proto.condition2.arg = condition->condition2;
        proto.condition2.funcs.encode = condition_encoder;
    } else {
        proto.condition.element.arg = condition->element;
        proto.condition.element.funcs.encode = element_encoder;
        proto.which_condition = Condition_element_tag;
    }
    return proto;
}


QueryResult fillQueryResult(queryResult_t *queryResult) {
    QueryResult proto = QueryResult_init_zero;
    proto.count = queryResult->count;
    proto.document_count = queryResult->count;
    for (int64_t i = 0; i < queryResult->count; i++) {
        proto.document[i] = fillDocument(queryResult->documents[i]);
    }
    return proto;
}

queryResult_t *getQueryResult(QueryResult proto) {
    queryResult_t *queryResult = createQueryResult();
    for (int64_t i = 0; i < proto.count; i++) {
        addQueryResult(queryResult, getDocument(proto.document[i]));
    }
    return queryResult;
}

Query fillQuery(query_t *query) {
    Query proto = Query_init_zero;
    proto.opType = (QueryType) query->type;
    strncpy(proto.documentName, query->documentName, 13);
    proto.limit = query->limit;
    if (query->condition) {
        proto.has_condition = true;
        proto.condition = fillCondition(query->condition);
    } else {
        proto.has_condition = false;
    }
    if (query->documents) {
        proto.result = fillQueryResult(query->documents);
        proto.has_result = true;
    } else {
        proto.has_result = false;
    }
    return proto;
}

query_t* getQuery(Query protoQuery, condition_t *condition) {
    query_t *query = createQuery((queryType_t) protoQuery.opType, protoQuery.documentName, condition,
                                 getQueryResult(protoQuery.result));
    return query;
}

int main() {
    condition_t *condition = createCondition(OP_AND,
                                             createCondition(OP_OR,
                                                             createCondition(OP_LT, stringElement("hi", "kkkk"), NULL),
                                                             createCondition(OP_NEQ, intElement("mewmew", 17), NULL)),
                                             createCondition(OP_AND,
                                                             createCondition(OP_EQ, stringElement("hui", "meow"), NULL),
                                                             createCondition(OP_NEQ, booleanElement("mewlalala", true),
                                                                             NULL)));
    queryResult_t *queryResult = createQueryResult();
    document_t *document = createDocument("mew");
    addElement(document, intElement("mew", 1));
    addElement(document, booleanElement("hi", true));
    addQueryResult(queryResult, document);
    document_t *document1 = createDocument("dococ");
    addElement(document1, stringElement("mewmewmew", "hihihiihihih"));
    addQueryResult(queryResult, document1);
    query_t *query = createQuery(SELECT, "name", condition, queryResult);

    Query proto = fillQuery(query);


    uint8_t buffer[256];
    size_t total_bytes_encoded = 0;
    pb_ostream_t ostream = pb_ostream_from_buffer(buffer, sizeof(buffer));
    if (!pb_encode(&ostream, Query_fields, &proto)) {
        const char *error = PB_GET_ERROR(&ostream);
        printf("pb_encode error: %s\n", error);
    } else {
        total_bytes_encoded = ostream.bytes_written;
        printf("Encoded size: %zu\n", total_bytes_encoded);
    }
    Query decodedData = {0};

    condition_t *conditionDecod = malloc(sizeof(condition_t));
    conditionDecod->condition1 = NULL;
    conditionDecod->condition2 = NULL;
    conditionDecod->element = NULL;
    Condition msg = Condition_init_zero;
    msg.opType.arg = conditionDecod;
    msg.opType.funcs.decode = opType_decode;
    msg.condition2.arg = conditionDecod;
    msg.condition2.funcs.decode = &condition_decoder;
    msg.condition.condition1.arg = conditionDecod;
    msg.condition.condition1.funcs.decode = &condition_decoder;
    decodedData.condition = msg;
    pb_istream_t istream = pb_istream_from_buffer(buffer, total_bytes_encoded);
    if (!pb_decode(&istream, Query_fields, &decodedData)) {
        const char *error = PB_GET_ERROR(&istream);
        printf("pb_decode error: %s\n", error);
    } else {
        printf("Bytes decoded: %zu\n", total_bytes_encoded - istream.bytes_left);

    }
    query_t * result = getQuery(decodedData, conditionDecod);

    printDocument(result->documents->documents[0]);
}

int main2() {
    document_t *document = createDocument("mew");
    addElement(document, intElement("mew", 1));
    addElement(document, booleanElement("hi", true));
    Document proto = Document_init_zero;
    proto.count = document->meta.count;
    strncpy(proto.name, document->meta.name, 13);
    proto.element_count = document->meta.count;
    for (int64_t i = 0; i < document->meta.count; i++) {
        strncpy(proto.element[i].key, document->elements[i]->key, 13);
        switch (document->elements[i]->type) {
            case INT: {
                proto.element[i].which_value = Element_integerValue_tag;
                proto.element[i].type = ElementType_INT;
                proto.element[i].value.integerValue = document->elements[i]->integerValue;
            }
            case BOOLEAN: {
                proto.element[i].which_value = Element_boolValue_tag;
                proto.element[i].type = ElementType_BOOLEAN;
                proto.element[i].value.boolValue = document->elements[i]->boolValue;
            }
        }
    }
    uint8_t buffer[256];
    size_t total_bytes_encoded = 0;
    pb_ostream_t ostream = pb_ostream_from_buffer(buffer, sizeof(buffer));
    if (!pb_encode(&ostream, Document_fields, &proto)) {
        const char *error = PB_GET_ERROR(&ostream);
        printf("pb_encode error: %s\n", error);
    } else {
        total_bytes_encoded = ostream.bytes_written;
        printf("Encoded size: %zu\n", total_bytes_encoded);
    }
    Document decodedData = {0};
    pb_istream_t istream = pb_istream_from_buffer(buffer, total_bytes_encoded);
    if (!pb_decode(&istream, Document_fields, &decodedData)) {
        const char *error = PB_GET_ERROR(&istream);
        printf("pb_decode error: %s\n", error);
    } else {
        printf("Bytes decoded: %zu\n", total_bytes_encoded - istream.bytes_left);

    }

}