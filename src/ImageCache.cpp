
#include <Geode/Geode.hpp>
#include "ImageCache.hpp"

ImageCache* ImageCache::instance = nullptr;

ImageCache::ImageCache(){
    m_imageDict = CCDictionary::create();
}

void ImageCache::addImage(CCImage* image, std::string key){
    if(!image) return;
    m_imageDict->setObject(image, key);
}

CCImage* ImageCache::getImage(std::string key){
    return static_cast<CCImage*>(m_imageDict->objectForKey(key));
}

void ImageCache::downloadImage(std::string url, std::string id, std::function<void(CCImage*, bool)> callback) {

    if (CCImage* img = ImageCache::get()->getImage(id)) {
        callback(img, true);
        return;
    };
        
    auto req = web::WebRequest();

    m_id++;

    int uid = m_id;
    m_listeners[uid].bind([id, callback, uid, this](web::WebTask::Event* e){
        if (auto res = e->getValue()){
            if (res->ok()) {
                auto data = res->data();
                std::thread imageThread = std::thread([data, id, callback, uid, this](){
                    CCImage* image = new CCImage();
                    if(!image->initWithImageData(const_cast<uint8_t*>(data.data()), data.size())) return;
                    
                    queueInMainThread([this, callback, image, id, uid] {
                        ImageCache::get()->addImage(image, id);
			            image->release();
                        callback(image, true);
                        m_listeners.erase(uid);
                    });
                });
                imageThread.detach();
            }
        }
        if (e->isCancelled()) {
            callback(nullptr, false);
            m_listeners.erase(uid);
        }
    });
    auto downloadTask = req.get(url);
    m_listeners[uid].setFilter(downloadTask);
}
