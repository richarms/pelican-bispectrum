#ifndef FACTORYREGISTRAR_H
#define FACTORYREGISTRAR_H

/**
 * @file FactoryRegistrar.h
 */

#include <boost/mpl/list.hpp>
#include <boost/mpl/size.hpp>
#include <boost/mpl/at.hpp>
#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/repetition/repeat.hpp>
#include <boost/preprocessor/repetition/enum.hpp>
#include <boost/preprocessor/repetition/enum_params.hpp>

#include <QtCore/QString>
#include <map>

#ifndef MAX_FACTORIES
/**
 * The MAX_FACTORIES macro determines how many factory templates
 * are generated by the preprocessor. <b>Practically, this means that there
 * can be a maxiumum of MAX_FACTORIES-1 constructor arguments for objects
 * created by any factory.</b>
 */
#define MAX_FACTORIES 4
#endif

/**
 * Helper macro (used only by other macros) to insert the types and
 * parameter names (P#) of the object constructor arguments.
 */
#define PARAM(z, n, data) \
typename boost::mpl::at_c<typename B::FactoryCtorList, n>::type P##n

namespace pelican {

/**
 * @ingroup c_utility
 *
 * @brief Blueprint for base class of registration objects.
 *
 * @details
 * This is the base class of all FactoryRegistrar objects. It holds the
 * static map containing the object names and pointers to the registrars
 * that can create those objects. When a FactoryRegistrar is initialised,
 * it updates the map held by this base class by calling declare() with
 * the object's type name. This ensures that it can be created by a factory
 * at a later point.
 *
 * To create an object, the factory should first call check() to ensure that
 * the object has been registered: it will throw an exception if not. The
 * factory should then call the pure virtual function create() to actually
 * construct the object, which is implemented by each FactoryRegistrar.
 *
 * The type map is available using the static function types().
 */
template<class B,
    int = boost::mpl::size<typename B::FactoryCtorList>::value
> struct RegBase {};

#define REGBASE(z, n, data) \
template<class B> struct RegBase<B, n> { \
    typedef std::map<QString, RegBase<B, n>*> TypeMap; \
\
    /* Virtual destructor to suppress compiler warnings */ \
    virtual ~RegBase<B, n>() {} \
\
    /* Performs a check to make sure the object's ID has been registered */ \
    static void check(const QString& id) { \
        if (!exists(id)) \
            throw QString("FactoryRegistrar: Unknown type '" + id + "'") \
            + QString("\nDid you include the object's header file?"); \
    } \
\
    /* Returns true if the object's ID has been registered, false if not */ \
    static bool exists(const QString& id) {return (types().count(id) > 0);} \
\
    /* Interface to create the object */ \
    virtual B* create(BOOST_PP_ENUM(n, PARAM, ~)) const = 0; \
\
    /* Declares an object with the given ID */ \
    static void declare(const QString& id, RegBase<B, n>* reg) { \
        types().insert(typename TypeMap::value_type(id, reg)); \
    } \
\
    /* Returns the object type map */ \
    static TypeMap& types() {static TypeMap types; return types;} \
};
BOOST_PP_REPEAT(MAX_FACTORIES, REGBASE, ~)
#undef REGBASE

/**
 * @brief Blueprint for registration objects of concrete classes.
 *
 * @details
 * The FactoryRegistrar is used to declare an object that can be created
 * by a factory, and is used by the PELICAN_DECLARE macro.
 * When constructed, the FactoryRegistrar updates the map held by the base
 * class by calling declare() with the supplied object ID and a pointer to
 * itself. This pointer is used by the factory to create the object using
 * the create() method, which actually does the job of construction.
 */
template<class B, class T,
    int n = boost::mpl::size<typename B::FactoryCtorList>::value
> struct FactoryRegistrar : public RegBase<B, n> {};

#define FACTORYREGISTRAR(z, n, data) \
template<class B, class T> \
struct FactoryRegistrar<B, T, n> : public RegBase<B, n> \
{ \
    /* Constructs a registrar for the object, which registers the ID */ \
    FactoryRegistrar(const QString& id) {RegBase<B, n>::declare(id, this);} \
\
    /* Creates a concrete object and returns its base class pointer */ \
    B* create(BOOST_PP_ENUM(n, PARAM, ~)) const { \
        return new T(BOOST_PP_ENUM_PARAMS(n, P)); \
    } \
};
BOOST_PP_REPEAT(MAX_FACTORIES, FACTORYREGISTRAR, ~)
#undef FACTORYREGISTRAR

/**
 * This is a convenience macro that is used to declare a concrete
 * object of type T, which has a base class of type B.
 */
#define PELICAN_DECLARE(B,T) \
namespace {pelican::FactoryRegistrar< B, T > BOOST_PP_CAT(fr_##T, __LINE__)(#T);}

/**
 * This is a convenience macro that is used to declare the types of
 * constructor arguments for concrete objects created by factories.
 * It should only be used next to the object's base class constructor.
 */
#define PELICAN_CONSTRUCT_TYPES(...) \
typedef boost::mpl::list< __VA_ARGS__ > FactoryCtorList;

/**
 * This is a convenience macro that is used to declare that the
 * constructed object takes no arguments.
 * It should only be used next to the object's base class constructor.
 */
#define PELICAN_CONSTRUCT_TYPES_EMPTY \
typedef boost::mpl::list<> FactoryCtorList;

} // namespace pelican

#endif // FACTORYREGISTRAR_H
