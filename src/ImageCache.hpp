#pragma once

#ifndef __IMAGECACHE_H
#define __IMAGECACHE_H

#include <Geode/Geode.hpp>
#include <Geode/utils/web.hpp>

using namespace geode::prelude;

class ImageCache {

protected:
    static ImageCache* instance;
public:

    Ref<CCDictionary> m_imageDict;

    ImageCache();
    void addImage(CCImage* image, std::string key);
    CCImage* getImage(std::string key);
    void downloadImage(std::string url, std::string id, std::function<void(CCImage*, bool)>);
    std::unordered_map<int, EventListener<web::WebTask>> m_listeners;
    int m_id;

    static ImageCache* get(){

        if (!instance) {
            instance = new ImageCache();
        };
        return instance;
    }
};


#endif