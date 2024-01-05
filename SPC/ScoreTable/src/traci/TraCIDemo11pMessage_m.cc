//
// Generated file, do not edit! Created by nedtool 5.7 from traci/TraCIDemo11pMessage.msg.
//

// Disable warnings about unused variables, empty switch stmts, etc:
#ifdef _MSC_VER
#  pragma warning(disable:4101)
#  pragma warning(disable:4065)
#endif

#if defined(__clang__)
#  pragma clang diagnostic ignored "-Wshadow"
#  pragma clang diagnostic ignored "-Wconversion"
#  pragma clang diagnostic ignored "-Wunused-parameter"
#  pragma clang diagnostic ignored "-Wc++98-compat"
#  pragma clang diagnostic ignored "-Wunreachable-code-break"
#  pragma clang diagnostic ignored "-Wold-style-cast"
#elif defined(__GNUC__)
#  pragma GCC diagnostic ignored "-Wshadow"
#  pragma GCC diagnostic ignored "-Wconversion"
#  pragma GCC diagnostic ignored "-Wunused-parameter"
#  pragma GCC diagnostic ignored "-Wold-style-cast"
#  pragma GCC diagnostic ignored "-Wsuggest-attribute=noreturn"
#  pragma GCC diagnostic ignored "-Wfloat-conversion"
#endif

#include <iostream>
#include <sstream>
#include <memory>
#include "TraCIDemo11pMessage_m.h"

namespace omnetpp {

// Template pack/unpack rules. They are declared *after* a1l type-specific pack functions for multiple reasons.
// They are in the omnetpp namespace, to allow them to be found by argument-dependent lookup via the cCommBuffer argument

// Packing/unpacking an std::vector
template<typename T, typename A>
void doParsimPacking(omnetpp::cCommBuffer *buffer, const std::vector<T,A>& v)
{
    int n = v.size();
    doParsimPacking(buffer, n);
    for (int i = 0; i < n; i++)
        doParsimPacking(buffer, v[i]);
}

template<typename T, typename A>
void doParsimUnpacking(omnetpp::cCommBuffer *buffer, std::vector<T,A>& v)
{
    int n;
    doParsimUnpacking(buffer, n);
    v.resize(n);
    for (int i = 0; i < n; i++)
        doParsimUnpacking(buffer, v[i]);
}

// Packing/unpacking an std::list
template<typename T, typename A>
void doParsimPacking(omnetpp::cCommBuffer *buffer, const std::list<T,A>& l)
{
    doParsimPacking(buffer, (int)l.size());
    for (typename std::list<T,A>::const_iterator it = l.begin(); it != l.end(); ++it)
        doParsimPacking(buffer, (T&)*it);
}

template<typename T, typename A>
void doParsimUnpacking(omnetpp::cCommBuffer *buffer, std::list<T,A>& l)
{
    int n;
    doParsimUnpacking(buffer, n);
    for (int i = 0; i < n; i++) {
        l.push_back(T());
        doParsimUnpacking(buffer, l.back());
    }
}

// Packing/unpacking an std::set
template<typename T, typename Tr, typename A>
void doParsimPacking(omnetpp::cCommBuffer *buffer, const std::set<T,Tr,A>& s)
{
    doParsimPacking(buffer, (int)s.size());
    for (typename std::set<T,Tr,A>::const_iterator it = s.begin(); it != s.end(); ++it)
        doParsimPacking(buffer, *it);
}

template<typename T, typename Tr, typename A>
void doParsimUnpacking(omnetpp::cCommBuffer *buffer, std::set<T,Tr,A>& s)
{
    int n;
    doParsimUnpacking(buffer, n);
    for (int i = 0; i < n; i++) {
        T x;
        doParsimUnpacking(buffer, x);
        s.insert(x);
    }
}

// Packing/unpacking an std::map
template<typename K, typename V, typename Tr, typename A>
void doParsimPacking(omnetpp::cCommBuffer *buffer, const std::map<K,V,Tr,A>& m)
{
    doParsimPacking(buffer, (int)m.size());
    for (typename std::map<K,V,Tr,A>::const_iterator it = m.begin(); it != m.end(); ++it) {
        doParsimPacking(buffer, it->first);
        doParsimPacking(buffer, it->second);
    }
}

template<typename K, typename V, typename Tr, typename A>
void doParsimUnpacking(omnetpp::cCommBuffer *buffer, std::map<K,V,Tr,A>& m)
{
    int n;
    doParsimUnpacking(buffer, n);
    for (int i = 0; i < n; i++) {
        K k; V v;
        doParsimUnpacking(buffer, k);
        doParsimUnpacking(buffer, v);
        m[k] = v;
    }
}

// Default pack/unpack function for arrays
template<typename T>
void doParsimArrayPacking(omnetpp::cCommBuffer *b, const T *t, int n)
{
    for (int i = 0; i < n; i++)
        doParsimPacking(b, t[i]);
}

template<typename T>
void doParsimArrayUnpacking(omnetpp::cCommBuffer *b, T *t, int n)
{
    for (int i = 0; i < n; i++)
        doParsimUnpacking(b, t[i]);
}

// Default rule to prevent compiler from choosing base class' doParsimPacking() function
template<typename T>
void doParsimPacking(omnetpp::cCommBuffer *, const T& t)
{
    throw omnetpp::cRuntimeError("Parsim error: No doParsimPacking() function for type %s", omnetpp::opp_typename(typeid(t)));
}

template<typename T>
void doParsimUnpacking(omnetpp::cCommBuffer *, T& t)
{
    throw omnetpp::cRuntimeError("Parsim error: No doParsimUnpacking() function for type %s", omnetpp::opp_typename(typeid(t)));
}

}  // namespace omnetpp

namespace {
template <class T> inline
typename std::enable_if<std::is_polymorphic<T>::value && std::is_base_of<omnetpp::cObject,T>::value, void *>::type
toVoidPtr(T* t)
{
    return (void *)(static_cast<const omnetpp::cObject *>(t));
}

template <class T> inline
typename std::enable_if<std::is_polymorphic<T>::value && !std::is_base_of<omnetpp::cObject,T>::value, void *>::type
toVoidPtr(T* t)
{
    return (void *)dynamic_cast<const void *>(t);
}

template <class T> inline
typename std::enable_if<!std::is_polymorphic<T>::value, void *>::type
toVoidPtr(T* t)
{
    return (void *)static_cast<const void *>(t);
}

}

namespace veins {

// forward
template<typename T, typename A>
std::ostream& operator<<(std::ostream& out, const std::vector<T,A>& vec);

// Template rule to generate operator<< for shared_ptr<T>
template<typename T>
inline std::ostream& operator<<(std::ostream& out,const std::shared_ptr<T>& t) { return out << t.get(); }

// Template rule which fires if a struct or class doesn't have operator<<
template<typename T>
inline std::ostream& operator<<(std::ostream& out,const T&) {return out;}

// operator<< for std::vector<T>
template<typename T, typename A>
inline std::ostream& operator<<(std::ostream& out, const std::vector<T,A>& vec)
{
    out.put('{');
    for(typename std::vector<T,A>::const_iterator it = vec.begin(); it != vec.end(); ++it)
    {
        if (it != vec.begin()) {
            out.put(','); out.put(' ');
        }
        out << *it;
    }
    out.put('}');

    char buf[32];
    sprintf(buf, " (size=%u)", (unsigned int)vec.size());
    out.write(buf, strlen(buf));
    return out;
}

Register_Class(TraCIDemo11pMessage)

TraCIDemo11pMessage::TraCIDemo11pMessage(const char *name, short kind) : ::veins::BaseFrame1609_4(name, kind)
{
}

TraCIDemo11pMessage::TraCIDemo11pMessage(const TraCIDemo11pMessage& other) : ::veins::BaseFrame1609_4(other)
{
    copy(other);
}

TraCIDemo11pMessage::~TraCIDemo11pMessage()
{
    delete [] this->scoreTable;
}

TraCIDemo11pMessage& TraCIDemo11pMessage::operator=(const TraCIDemo11pMessage& other)
{
    if (this == &other) return *this;
    ::veins::BaseFrame1609_4::operator=(other);
    copy(other);
    return *this;
}

void TraCIDemo11pMessage::copy(const TraCIDemo11pMessage& other)
{
    this->vehID = other.vehID;
    this->vehSpeed = other.vehSpeed;
    this->timestamp = other.timestamp;
    this->vehType = other.vehType;
    delete [] this->scoreTable;
    this->scoreTable = (other.scoreTable_arraysize==0) ? nullptr : new omnetpp::opp_string[other.scoreTable_arraysize];
    scoreTable_arraysize = other.scoreTable_arraysize;
    for (size_t i = 0; i < scoreTable_arraysize; i++) {
        this->scoreTable[i] = other.scoreTable[i];
    }
    this->senderAddress = other.senderAddress;
    this->serial = other.serial;
}

void TraCIDemo11pMessage::parsimPack(omnetpp::cCommBuffer *b) const
{
    ::veins::BaseFrame1609_4::parsimPack(b);
    doParsimPacking(b,this->vehID);
    doParsimPacking(b,this->vehSpeed);
    doParsimPacking(b,this->timestamp);
    doParsimPacking(b,this->vehType);
    b->pack(scoreTable_arraysize);
    doParsimArrayPacking(b,this->scoreTable,scoreTable_arraysize);
    doParsimPacking(b,this->senderAddress);
    doParsimPacking(b,this->serial);
}

void TraCIDemo11pMessage::parsimUnpack(omnetpp::cCommBuffer *b)
{
    ::veins::BaseFrame1609_4::parsimUnpack(b);
    doParsimUnpacking(b,this->vehID);
    doParsimUnpacking(b,this->vehSpeed);
    doParsimUnpacking(b,this->timestamp);
    doParsimUnpacking(b,this->vehType);
    delete [] this->scoreTable;
    b->unpack(scoreTable_arraysize);
    if (scoreTable_arraysize == 0) {
        this->scoreTable = nullptr;
    } else {
        this->scoreTable = new omnetpp::opp_string[scoreTable_arraysize];
        doParsimArrayUnpacking(b,this->scoreTable,scoreTable_arraysize);
    }
    doParsimUnpacking(b,this->senderAddress);
    doParsimUnpacking(b,this->serial);
}

const char * TraCIDemo11pMessage::getVehID() const
{
    return this->vehID.c_str();
}

void TraCIDemo11pMessage::setVehID(const char * vehID)
{
    this->vehID = vehID;
}

const char * TraCIDemo11pMessage::getVehSpeed() const
{
    return this->vehSpeed.c_str();
}

void TraCIDemo11pMessage::setVehSpeed(const char * vehSpeed)
{
    this->vehSpeed = vehSpeed;
}

const char * TraCIDemo11pMessage::getTimestamp() const
{
    return this->timestamp.c_str();
}

void TraCIDemo11pMessage::setTimestamp(const char * timestamp)
{
    this->timestamp = timestamp;
}

const char * TraCIDemo11pMessage::getVehType() const
{
    return this->vehType.c_str();
}

void TraCIDemo11pMessage::setVehType(const char * vehType)
{
    this->vehType = vehType;
}

size_t TraCIDemo11pMessage::getScoreTableArraySize() const
{
    return scoreTable_arraysize;
}

const char * TraCIDemo11pMessage::getScoreTable(size_t k) const
{
    if (k >= scoreTable_arraysize) throw omnetpp::cRuntimeError("Array of size scoreTable_arraysize indexed by %lu", (unsigned long)k);
    return this->scoreTable[k].c_str();
}

void TraCIDemo11pMessage::setScoreTableArraySize(size_t newSize)
{
    omnetpp::opp_string *scoreTable2 = (newSize==0) ? nullptr : new omnetpp::opp_string[newSize];
    size_t minSize = scoreTable_arraysize < newSize ? scoreTable_arraysize : newSize;
    for (size_t i = 0; i < minSize; i++)
        scoreTable2[i] = this->scoreTable[i];
    delete [] this->scoreTable;
    this->scoreTable = scoreTable2;
    scoreTable_arraysize = newSize;
}

void TraCIDemo11pMessage::setScoreTable(size_t k, const char * scoreTable)
{
    if (k >= scoreTable_arraysize) throw omnetpp::cRuntimeError("Array of size  indexed by %lu", (unsigned long)k);
    this->scoreTable[k] = scoreTable;
}

void TraCIDemo11pMessage::insertScoreTable(size_t k, const char * scoreTable)
{
    if (k > scoreTable_arraysize) throw omnetpp::cRuntimeError("Array of size  indexed by %lu", (unsigned long)k);
    size_t newSize = scoreTable_arraysize + 1;
    omnetpp::opp_string *scoreTable2 = new omnetpp::opp_string[newSize];
    size_t i;
    for (i = 0; i < k; i++)
        scoreTable2[i] = this->scoreTable[i];
    scoreTable2[k] = scoreTable;
    for (i = k + 1; i < newSize; i++)
        scoreTable2[i] = this->scoreTable[i-1];
    delete [] this->scoreTable;
    this->scoreTable = scoreTable2;
    scoreTable_arraysize = newSize;
}

void TraCIDemo11pMessage::insertScoreTable(const char * scoreTable)
{
    insertScoreTable(scoreTable_arraysize, scoreTable);
}

void TraCIDemo11pMessage::eraseScoreTable(size_t k)
{
    if (k >= scoreTable_arraysize) throw omnetpp::cRuntimeError("Array of size  indexed by %lu", (unsigned long)k);
    size_t newSize = scoreTable_arraysize - 1;
    omnetpp::opp_string *scoreTable2 = (newSize == 0) ? nullptr : new omnetpp::opp_string[newSize];
    size_t i;
    for (i = 0; i < k; i++)
        scoreTable2[i] = this->scoreTable[i];
    for (i = k; i < newSize; i++)
        scoreTable2[i] = this->scoreTable[i+1];
    delete [] this->scoreTable;
    this->scoreTable = scoreTable2;
    scoreTable_arraysize = newSize;
}

const LAddress::L2Type& TraCIDemo11pMessage::getSenderAddress() const
{
    return this->senderAddress;
}

void TraCIDemo11pMessage::setSenderAddress(const LAddress::L2Type& senderAddress)
{
    this->senderAddress = senderAddress;
}

int TraCIDemo11pMessage::getSerial() const
{
    return this->serial;
}

void TraCIDemo11pMessage::setSerial(int serial)
{
    this->serial = serial;
}

class TraCIDemo11pMessageDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertynames;
    enum FieldConstants {
        FIELD_vehID,
        FIELD_vehSpeed,
        FIELD_timestamp,
        FIELD_vehType,
        FIELD_scoreTable,
        FIELD_senderAddress,
        FIELD_serial,
    };
  public:
    TraCIDemo11pMessageDescriptor();
    virtual ~TraCIDemo11pMessageDescriptor();

    virtual bool doesSupport(omnetpp::cObject *obj) const override;
    virtual const char **getPropertyNames() const override;
    virtual const char *getProperty(const char *propertyname) const override;
    virtual int getFieldCount() const override;
    virtual const char *getFieldName(int field) const override;
    virtual int findField(const char *fieldName) const override;
    virtual unsigned int getFieldTypeFlags(int field) const override;
    virtual const char *getFieldTypeString(int field) const override;
    virtual const char **getFieldPropertyNames(int field) const override;
    virtual const char *getFieldProperty(int field, const char *propertyname) const override;
    virtual int getFieldArraySize(void *object, int field) const override;

    virtual const char *getFieldDynamicTypeString(void *object, int field, int i) const override;
    virtual std::string getFieldValueAsString(void *object, int field, int i) const override;
    virtual bool setFieldValueAsString(void *object, int field, int i, const char *value) const override;

    virtual const char *getFieldStructName(int field) const override;
    virtual void *getFieldStructValuePointer(void *object, int field, int i) const override;
};

Register_ClassDescriptor(TraCIDemo11pMessageDescriptor)

TraCIDemo11pMessageDescriptor::TraCIDemo11pMessageDescriptor() : omnetpp::cClassDescriptor(omnetpp::opp_typename(typeid(veins::TraCIDemo11pMessage)), "veins::BaseFrame1609_4")
{
    propertynames = nullptr;
}

TraCIDemo11pMessageDescriptor::~TraCIDemo11pMessageDescriptor()
{
    delete[] propertynames;
}

bool TraCIDemo11pMessageDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<TraCIDemo11pMessage *>(obj)!=nullptr;
}

const char **TraCIDemo11pMessageDescriptor::getPropertyNames() const
{
    if (!propertynames) {
        static const char *names[] = {  nullptr };
        omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
        const char **basenames = basedesc ? basedesc->getPropertyNames() : nullptr;
        propertynames = mergeLists(basenames, names);
    }
    return propertynames;
}

const char *TraCIDemo11pMessageDescriptor::getProperty(const char *propertyname) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : nullptr;
}

int TraCIDemo11pMessageDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 7+basedesc->getFieldCount() : 7;
}

unsigned int TraCIDemo11pMessageDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeFlags(field);
        field -= basedesc->getFieldCount();
    }
    static unsigned int fieldTypeFlags[] = {
        FD_ISEDITABLE,    // FIELD_vehID
        FD_ISEDITABLE,    // FIELD_vehSpeed
        FD_ISEDITABLE,    // FIELD_timestamp
        FD_ISEDITABLE,    // FIELD_vehType
        FD_ISARRAY | FD_ISEDITABLE,    // FIELD_scoreTable
        0,    // FIELD_senderAddress
        FD_ISEDITABLE,    // FIELD_serial
    };
    return (field >= 0 && field < 7) ? fieldTypeFlags[field] : 0;
}

const char *TraCIDemo11pMessageDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldName(field);
        field -= basedesc->getFieldCount();
    }
    static const char *fieldNames[] = {
        "vehID",
        "vehSpeed",
        "timestamp",
        "vehType",
        "scoreTable",
        "senderAddress",
        "serial",
    };
    return (field >= 0 && field < 7) ? fieldNames[field] : nullptr;
}

int TraCIDemo11pMessageDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    int base = basedesc ? basedesc->getFieldCount() : 0;
    if (fieldName[0] == 'v' && strcmp(fieldName, "vehID") == 0) return base+0;
    if (fieldName[0] == 'v' && strcmp(fieldName, "vehSpeed") == 0) return base+1;
    if (fieldName[0] == 't' && strcmp(fieldName, "timestamp") == 0) return base+2;
    if (fieldName[0] == 'v' && strcmp(fieldName, "vehType") == 0) return base+3;
    if (fieldName[0] == 's' && strcmp(fieldName, "scoreTable") == 0) return base+4;
    if (fieldName[0] == 's' && strcmp(fieldName, "senderAddress") == 0) return base+5;
    if (fieldName[0] == 's' && strcmp(fieldName, "serial") == 0) return base+6;
    return basedesc ? basedesc->findField(fieldName) : -1;
}

const char *TraCIDemo11pMessageDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeString(field);
        field -= basedesc->getFieldCount();
    }
    static const char *fieldTypeStrings[] = {
        "string",    // FIELD_vehID
        "string",    // FIELD_vehSpeed
        "string",    // FIELD_timestamp
        "string",    // FIELD_vehType
        "string",    // FIELD_scoreTable
        "veins::LAddress::L2Type",    // FIELD_senderAddress
        "int",    // FIELD_serial
    };
    return (field >= 0 && field < 7) ? fieldTypeStrings[field] : nullptr;
}

const char **TraCIDemo11pMessageDescriptor::getFieldPropertyNames(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldPropertyNames(field);
        field -= basedesc->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

const char *TraCIDemo11pMessageDescriptor::getFieldProperty(int field, const char *propertyname) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldProperty(field, propertyname);
        field -= basedesc->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

int TraCIDemo11pMessageDescriptor::getFieldArraySize(void *object, int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldArraySize(object, field);
        field -= basedesc->getFieldCount();
    }
    TraCIDemo11pMessage *pp = (TraCIDemo11pMessage *)object; (void)pp;
    switch (field) {
        case FIELD_scoreTable: return pp->getScoreTableArraySize();
        default: return 0;
    }
}

const char *TraCIDemo11pMessageDescriptor::getFieldDynamicTypeString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldDynamicTypeString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    TraCIDemo11pMessage *pp = (TraCIDemo11pMessage *)object; (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string TraCIDemo11pMessageDescriptor::getFieldValueAsString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldValueAsString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    TraCIDemo11pMessage *pp = (TraCIDemo11pMessage *)object; (void)pp;
    switch (field) {
        case FIELD_vehID: return oppstring2string(pp->getVehID());
        case FIELD_vehSpeed: return oppstring2string(pp->getVehSpeed());
        case FIELD_timestamp: return oppstring2string(pp->getTimestamp());
        case FIELD_vehType: return oppstring2string(pp->getVehType());
        case FIELD_scoreTable: return oppstring2string(pp->getScoreTable(i));
        case FIELD_senderAddress: {std::stringstream out; out << pp->getSenderAddress(); return out.str();}
        case FIELD_serial: return long2string(pp->getSerial());
        default: return "";
    }
}

bool TraCIDemo11pMessageDescriptor::setFieldValueAsString(void *object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->setFieldValueAsString(object,field,i,value);
        field -= basedesc->getFieldCount();
    }
    TraCIDemo11pMessage *pp = (TraCIDemo11pMessage *)object; (void)pp;
    switch (field) {
        case FIELD_vehID: pp->setVehID((value)); return true;
        case FIELD_vehSpeed: pp->setVehSpeed((value)); return true;
        case FIELD_timestamp: pp->setTimestamp((value)); return true;
        case FIELD_vehType: pp->setVehType((value)); return true;
        case FIELD_scoreTable: pp->setScoreTable(i,(value)); return true;
        case FIELD_serial: pp->setSerial(string2long(value)); return true;
        default: return false;
    }
}

const char *TraCIDemo11pMessageDescriptor::getFieldStructName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructName(field);
        field -= basedesc->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    };
}

void *TraCIDemo11pMessageDescriptor::getFieldStructValuePointer(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructValuePointer(object, field, i);
        field -= basedesc->getFieldCount();
    }
    TraCIDemo11pMessage *pp = (TraCIDemo11pMessage *)object; (void)pp;
    switch (field) {
        case FIELD_senderAddress: return toVoidPtr(&pp->getSenderAddress()); break;
        default: return nullptr;
    }
}

} // namespace veins

