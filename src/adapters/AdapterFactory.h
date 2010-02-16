#ifndef ADAPTERFACTOR_H
#define ADAPTERFACTOR_H

#include <QString>
#include <QVector>
#include "utility/Config.h"

/**
 * @file AdapterFactory.h
 */

namespace pelican {

class ConfigNode;
class AbstractAdapter;

/**
 * @class AdapterFactory
 *  
 * @brief
 * Factory to create pre-configured adapters.
 * 
 * @details
 * Instantiates adapter objects and configures them with details taken
 * from the appropriate configuration file and/or command line.
 * The adapters are owned by the factory and will be destroyed with
 * the factory.
 */

class AdapterFactory
{
    public:
        /// Adapter factory constructor.
        AdapterFactory(const Config *config);

        /// Module factory destructor.
        ~AdapterFactory();

        /// Creates a new module.
        AbstractAdapter* create(const QString& type, const QString& name = QString());

        /// Returns the tree node address that marks the start
        /// of the module configuration block.
        Config::TreeAddress_t configRoot() const { return _configRoot; }

    private:
        /// Creates a new adapter.
        AbstractAdapter* _create(const QString& type, const ConfigNode& config);

    private:
        const Config* _config; ///< Pointer to the configuration object.
        Config::TreeAddress_t _configRoot;
        QVector<AbstractAdapter*> _adapters; ///< Holds pointers to the created adapters.
};

} // namespace pelican

#endif // ADAPTERFACTORY_H