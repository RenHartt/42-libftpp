#pragma once

#include "data_buffer.hpp"

class Memento {
public:
    class Snapshot : public DataBuffer {
        friend Memento;
    };

    Snapshot save() const {
        Snapshot snapshot;
        this->_saveToSnapshot(snapshot);
        return snapshot;
    }

    void load(Snapshot& state) {
        this->_loadFromSnapshot(state);
    }

protected:
    virtual void _saveToSnapshot(Snapshot&) const = 0;
    virtual void _loadFromSnapshot(Snapshot&) = 0;
};