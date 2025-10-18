#include <complex>
#include <map>
#include<stack>
#include <memory>
#include "third_party/Image_Class.h"
#include <stdexcept>
#include <vector>
#include<cmath>
#include<unordered_map>
#define M_PI 3.14159265359
#include <variant>
#include "iostream"
#include <filesystem>
#include<algorithm>
namespace fs = std::filesystem;
using namespace std;
# define ll long long


struct FilterParam {
    string name;          // اسم المتغير (مثلاً "radius" أو "angle")
    string type;          // نوعه ("int" أو "float" أو "string" أو "color")
    string defaultValue;  // قيمة افتراضية كنص
    variant<int, double, string, bool> value;
    double minValue = 0;  // الحد الأدنى (لـ float/int فقط)
    double maxValue = 100; // الحد الأقصى
};


struct HSV {
    float h, s, v;
};
struct RGB {
    int R, G, B;
};

class Filter
{
protected:
    Image& image;
    double threshold = 127;
    unordered_map<string, FilterParam> params;
    // static string id;
    // string outputFolderPath = "../../output/";
public:
    Filter(Image& img) : image(img) {};
    virtual void apply() = 0;
    virtual vector<FilterParam> getNeeds() { return {}; };

    virtual void setParam(const std::string &name, double val) {
        params[name].value = val;
    }

    virtual void setParam(const std::string &name, int val) {
        params[name].value = val;
    }

    virtual void setParam(const std::string &name, bool val) {
        params[name].value = val;
    }

    virtual void setParam(const std::string &name, const std::string &val) {
        params[name].value = val;
    }

    template<typename T>
    T getParam(const string &name, const T &defaultVal = T()) const {
        auto it = params.find(name);
        if (it != params.end()) {
            if (auto val = get_if<T>(&it->second))
                return *val;
        }
        return defaultVal;
    }

    virtual string getName() = 0;

    double computeThreshold() {
        vector<int> intensities(image.width * image.height);

        for (int x = 0; x < image.width; x++) {
            for (int y = 0; y < image.height; y++) {
                int r = image(x, y, 0);
                int g = image(x, y, 1);
                int b = image(x, y, 2);
                intensities.push_back(((r + g + b) / 3));
            }
        }

        double sum = 0;
        for (int& val : intensities) sum += val;

        double mean = sum / intensities.size();

        double variance = 0;
        for (int& val : intensities) variance += pow(val - mean, 2);

        variance /= intensities.size();

        double standardDeviation = sqrt(variance);

        threshold = mean + 0.6 * standardDeviation;

        return threshold;
    }

    double getThreshold() {
        return threshold;
    }

    bool isInBound(int x = 0, int y = 0) {
        bool output = true;
        if (x < 0 || x >= image.width) output = false;
        if (y < 0 || y >= image.height) output = false;

        return output;
    }
    static void resizeImage(Image& img, int newW, int newH) {
        Image resizedImage(newW, newH);

        double xRatio = static_cast<double>(img.width) / newW;
        double yRatio = static_cast<double>(img.height) / newH;

        for (int x = 0; x < newW; x++) {
            for (int y = 0; y < newH; y++) {
                int nearestX = static_cast<int>(x * xRatio);
                int nearestY = static_cast<int>(y * yRatio);

                nearestX = min(nearestX, img.width - 1);
                nearestY = min(nearestY, img.height - 1);

                for (int k = 0; k < img.channels; k++) {
                    resizedImage(x, y, k) = img(nearestX, nearestY, k);
                }
            }
        }

        img = resizedImage;
    }
    // static string getId() {};
};

class Sunlight : public Filter
{
public:
    Sunlight(Image& img) : Filter(img) {};
    string getName() { return "Sunlight"; };
    static string getId() { return "13"; };
    void apply()
    {
        for (int i = 0; i < image.width; i++)
        {
            for (int j = 0; j < image.height; j++)
            {
                double r = 1.1 * image(i, j, 0);
                if (r > 255)
                {
                    r = 255;
                }
                image(i, j, 0) = r;
                double g = 1.2 * image(i, j, 1);
                if (g > 255)
                {
                    g = 255;
                }
                image(i, j, 1) = g;
                double b = 0.7 * image(i, j, 2);
                if (b < 0)
                {
                    b = 0;
                }
                image(i, j, 2) = b;
            }
        }
    }
    vector<FilterParam> getNeeds() override {return {};};
};
class Night : public Filter
{
public:
    Night(Image& img) : Filter(img) {};
    string getName() { return "Night"; };
    static string getId() { return "16"; };
    void apply()
    {
        for (int i = 0; i < image.width; i++)
        {
            for (int j = 0; j < image.height; j++)
            {
                double r = 1.4 * image(i, j, 0);
                if (r > 255)
                {
                    r = 255;
                }
                image(i, j, 0) = r;
                double g = 0.7 * image(i, j, 1);
                if (g < 0)
                {
                    g = 0;
                }
                image(i, j, 1) = g;
                double b = 1.6 * image(i, j, 2);
                if (b > 255)
                {
                    b = 255;
                }
                image(i, j, 2) = b;
            }
        }

    }
    vector<FilterParam> getNeeds() override {return {};};
};
class Blur : public Filter {
    int radius;

public:
    Blur(Image& img, int r = 10) : Filter(img), radius(r) {};
    string getName() { return "Blur"; };
    static string getId() { return "12"; };
    void setParam(const std::string& name, double value) {
        if (name == "Blur Strength (0:100)") radius = value;
    }

    vector<FilterParam> getNeeds() {
        return { {"Blur Strength (0:100)", "float", "5", 0.0, 100.0} };
    }

    void Prefix_sum(Image& image, vector<vector<ll>>& prefixR, vector<vector<ll>>& prefixG, vector<vector<ll>>& prefixB)
    {
        for (int i = 1; i <= image.width; i++)
        {
            for (int j = 1; j <= image.height; j++)
            {
                ll r = image(i - 1, j - 1, 0);
                ll g = image(i - 1, j - 1, 1);
                ll b = image(i - 1, j - 1, 2);
                prefixR[i][j] = r + prefixR[i - 1][j] + prefixR[i][j - 1] - prefixR[i - 1][j - 1];
                prefixG[i][j] = g + prefixG[i - 1][j] + prefixG[i][j - 1] - prefixG[i - 1][j - 1];
                prefixB[i][j] = b + prefixB[i - 1][j] + prefixB[i][j - 1] - prefixB[i - 1][j - 1];
            }
        }
    }
    void apply() override
    {
        try
        {
            Image Blured_image(image.width, image.height);
            vector<vector<ll>> PrefixR(image.width + 1, vector<ll>(image.height + 1, 0));
            vector<vector<ll>> PrefixG(image.width + 1, vector<ll>(image.height + 1, 0));
            vector<vector<ll>> PrefixB(image.width + 1, vector<ll>(image.height + 1, 0));
            Prefix_sum(image, PrefixR, PrefixG, PrefixB);
            for (int i = 0; i < image.width; i++)
            {
                for (int j = 0; j < image.height; j++)
                {
                    int x1, x2, y1, y2;

                    x1 = max(0, i - radius);
                    x2 = min(image.width - 1, i + radius);
                    y1 = max(0, j - radius);
                    y2 = min(image.height - 1, j + radius);

                    x1++, x2++, y1++, y2++;


                    ll SUM_R, SUM_G, SUM_B;
                    SUM_R = PrefixR[x2][y2] + PrefixR[x1 - 1][y1 - 1] - PrefixR[x2][y1 - 1] - PrefixR[x1 - 1][y2];
                    SUM_G = PrefixG[x2][y2] + PrefixG[x1 - 1][y1 - 1] - PrefixG[x2][y1 - 1] - PrefixG[x1 - 1][y2];
                    SUM_B = PrefixB[x2][y2] + PrefixB[x1 - 1][y1 - 1] - PrefixB[x2][y1 - 1] - PrefixB[x1 - 1][y2];


                    ll A = (2 * radius + 1) * (2 * radius + 1);


                    Blured_image(i, j, 0) = static_cast<unsigned char>(SUM_R / A);
                    Blured_image(i, j, 1) = static_cast<unsigned char>(SUM_G / A);
                    Blured_image(i, j, 2) = static_cast<unsigned char>(SUM_B / A);
                }
            }
            image = Blured_image;
        }
        catch (const std::exception& e)
        {
            std::cerr << "Error: " << e.what() << std::endl;
        }
    }
};
class Skewing : public Filter
{
    int angle;
public:
    Skewing(Image& img) : Filter(img) {};
    string getName() { return "Horizontal Skew"; };
    static string getId() { return "18"; };
    void apply()
    {
        try
        {
            double tan_angle = tan(angle * M_PI / 180);
            double slope = -tan_angle;
            int new_width = image.width + abs(image.height * slope);
            int checker = 0;
            if (slope < 0)
            {
                checker = abs(image.height * slope);
            }

            Image Skewed_image(new_width, image.height);
            for (int x = 0; x < image.width; x++)
            {
                for (int y = 0; y < image.height; y++)
                {
                    double X = x + (slope * y) + checker;
                    /*if (X < 0)  X += abs(image.height * slope); */
                    for (int k = 0; k < 3; k++)
                    {
                        if (X >= 0 && X < new_width)
                        {
                            Skewed_image(X, y, k) = image(x, y, k);
                        }
                    }
                }
            }
            image = Skewed_image;
        }
        catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
            throw;
        }
    }

    vector<FilterParam> getNeeds() {
        return {
            {"Skew Angle (-100:100)", "float", "10",-45, 45}
        };
    }
    void setParam(const std::string& name, double value) {
        if (name == "Skew Angle (-100:100)") angle = value;
    }
};
class OldTV : public Filter
{
public:
    OldTV(Image& img) : Filter(img) {};
    string getName() { return "Old TV"; };
    static string getId() { return "15"; };
    void apply() override
    {
        try {
            for (int i = 0; i < image.height - 1; i += 2)
            {
                for (int j = 0; j < image.width; j++)
                {
                    for (int k = 0; k < 3;k++)
                    {
                        image(j, i, k) = 0.5 * image(j, i, k);
                    }
                }
            }
        }

        catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
            throw;
        }
    }
    vector<FilterParam> getNeeds() override {return {};};
};
class GreyScale : public Filter
{
public:
    GreyScale(Image& img) : Filter(img) {};
    string getName() { return "Grey Scale"; };
    static string getId() { return "1"; };
    void apply() override
    {
        try
        {
            for (int i = 0; i < image.width; i++)
            {
                for (int j = 0; j < image.height; j++)
                {

                    unsigned int avg = 0;

                    for (int k = 0; k < 3; k++)
                    {
                        avg += image(i, j, k);
                    }

                    avg /= image.channels; // average
                    for (int k = 0; k < image.channels; k++)
                    {
                        image(i, j, k) = avg;
                    }
                }
            }
        }
        catch (const exception& e)
        {
            cerr << "Error: " << e.what() << endl;
            throw;
        }
    }
    vector<FilterParam> getNeeds() override {return {};};
};
class WhiteAndBlack : public Filter
{
public:
    WhiteAndBlack(Image& img) : Filter(img) {};
    string getName() { return "White and Black"; };
    static string getId() { return "2"; };
    void apply() override
    {
        computeThreshold();
        for (int i = 0; i < image.width; i++)
        {
            for (int j = 0; j < image.height; j++)
            {
                unsigned short avg = 0;
                for (int k = 0; k < image.channels; k++)
                {
                    avg += image(i, j, k);
                }

                avg /= image.channels;

                for (int k = 0; k < image.channels; k++)
                {
                    image(i, j, k) = (avg >= (threshold) ? 255 : 0);
                }
            }
        }
    };
    vector<FilterParam> getNeeds() override {return {};};
};
class Merge : public Filter
{
    Image overlay;
    int mergeType = 1;

public:
    Merge(Image& img) : Filter(img) {};
    string getName() { return "Merge"; };
    static string getId() { return "4"; };

    vector<FilterParam> getNeeds() {
        return {
            {"Merge Type", "choice", "1",1, 2}
        };
    }
    void setParam(const std::string& name, double value) {
        if (name == "Merge Type") mergeType = (int)value;
    }
    void apply() override
    {
        Image& ov = this->overlay;
        Image& base = image;
        try
        {
            int height;
            int width;

            if (base.width == overlay.width && base.height == overlay.height)
            {
                height = base.height;
                width = base.width;
                for (int i = 0; i < width; i++)
                {
                    for (int j = 0; j < height; j++)
                    {
                        for (int k = 0; k < 3; k++)
                        {
                            base(i, j, k) = (base(i, j, k) + overlay(i, j, k)) / 2;
                        }
                    }
                }
            }
            else
            {
                switch (mergeType)
                {
                case 1:
                { // max
                    /* Max */
                    width = std::max(base.width, overlay.width);
                    height = std::max(base.height, overlay.height);
                    resizeImage(base, width, height);
                    resizeImage(overlay, width, height);
                    for (int i = 0; i < width; i++)
                    {
                        for (int j = 0; j < height; j++)
                        {
                            for (int k = 0; k < 3; k++)
                            {
                                base(i, j, k) = (base(i, j, k) + overlay(i, j, k)) / 2;
                            }
                        }
                    }
                    break;
                }
                case 2:
                { // both
                    Image img(std::max(base.width, overlay.width),
                              std::max(base.height, overlay.height));
                    for (int i = 0; i < base.width; i++)
                    {
                        for (int j = 0; j < base.height; j++)
                        {
                            for (int k = 0; k < 3; k++)
                            {
                                img(i, j, k) = base(i, j, k);
                            }
                        }
                    }
                    for (int i = 0; i < overlay.width; i++)
                    {
                        for (int j = 0; j < overlay.height; j++)
                        {
                            for (int k = 0; k < 3; k++)
                            {
                                if (i < base.width && j < base.height)
                                {
                                    img(i, j, k) = (img(i, j, k) + overlay(i, j, k)) / 2;
                                }
                                else
                                {
                                    img(i, j, k) = overlay(i, j, k);
                                }
                            }
                        }
                    }

                    base = img;
                }
                default:
                    break;
                }
            }
        }
        catch (const std::exception& e)
        {
            std::cerr << "Error: " << e.what() << std::endl;
            throw;
        }
    }
};
class Flip : public Filter
{
    char dir = 'h';
public:
    Flip(Image& img) : Filter(img) {};
    string getName() { return "Flip"; };
    static string getId() { return "5"; };

    vector<FilterParam> getNeeds() {
        return {
            {"Direction (0=Vertical, 1=Horizontal)", "int", "0",0, 1} // 0=Vertical, 1=Horizontal
        };
    }

   void setParam(const std::string& name, double value) {
        if (name == "Direction (0=Vertical, 1=Horizontal)")
            dir = (value == 0) ? 'v' : 'h';
    }
    void apply() override
    {
        if (dir == 'h')
        {
            for (int i = 0; i < image.height; i++)
            {
                for (int j = 0; j < image.width / 2; j++)
                {
                    int tempChannels[3] = { 0 };
                    for (int k = 0; k < image.channels; k++)
                    {
                        tempChannels[k] = image(j, i, k);
                    }

                    for (int k = 0; k < image.channels; k++)
                    {
                        image(j, i, k) = image(image.width - j - 1, i, k);
                    }

                    for (int k = 0; k < image.channels; k++)
                    {
                        image(image.width - j - 1, i, k) = tempChannels[k];
                    }
                }
            }
        }
        else
        {
            for (int j = 0; j < image.width; j++)
            {
                for (int i = 0; i < image.height / 2; i++)
                {
                    int tempChannels[3] = { 0 };
                    for (int k = 0; k < image.channels; k++)
                    {
                        tempChannels[k] = image(j, i, k);
                    }

                    for (int k = 0; k < image.channels; k++)
                    {
                        image(j, i, k) = image(j, image.height - i - 1, k);
                    }

                    for (int k = 0; k < image.channels; k++)
                    {
                        image(j, image.height - i - 1, k) = tempChannels[k];
                    }
                }
            }
        }
    }

};
class Invert : public Filter {
public:
    Invert(Image& img) : Filter(img) {};
    void apply() override {
        for (int i = 0; i < image.width; i++) {
            for (int j = 0; j < image.height; j++) {
                for (int k = 0; k < 3; k++) {
                    image(i, j, k) = 255 - image(i, j, k);
                }
            }
        }
    }
    string getName() { return "Invert"; };
    static string getId() { return "3"; };

    vector<FilterParam> getNeeds() override {return {};};

};
class Rotate : public Filter
{
    int angle;
public:
    Rotate(Image& img) : Filter(img) {};
    string getName() { return "Rotate"; };
    static string getId() { return "6"; };

    void apply() override
    {
        try
        {
            Image rotated_image;
            /*angle = 360 - angle; */
            switch (angle)
            {
            case 90:
            case 270:
                rotated_image = Image(image.height, image.width);
                break;

            case 180:
                rotated_image = Image(image.width, image.height);
                break;
            }

            double radian = angle * M_PI / 180.0;
            double cos_Angle = cos(radian);
            double sin_Angle = sin(radian);

            int cx = image.width / 2;
            int cy = image.height / 2;

            int ncx = rotated_image.width / 2;
            int ncy = rotated_image.height / 2;

            for (int x = 0; x < rotated_image.width; x++)
            {
                for (int y = 0; y < rotated_image.height; y++)
                {
                    int X = cx + (x - ncx) * cos_Angle + (y - ncy) * sin_Angle;
                    int Y = cy - (x - ncx) * sin_Angle + (y - ncy) * cos_Angle;

                    if (X >= 0 && X < image.width && Y >= 0 && Y < image.height)
                    {
                        for (int k = 0; k < 3; k++)
                        {
                            rotated_image(x, y, k) = image(X, Y, k);
                        }
                    }
                }
            }

            image = rotated_image;
        }
        catch (const std::exception& e)
        {
            std::cerr << "Error: " << e.what() << std::endl;
            throw;
        }
    }

    vector<FilterParam> getNeeds() {
        return {
            {"Rotation Angle (90 / 180 / 270)", "int", "90",90, 270}
        };
    }
    void setParam(const std::string& name, double value) {
        if (name == "Rotation Angle (90 / 180 / 270)") angle = (int)value;
    }
};
class Brightness : public Filter
{
    double value;
public:
    Brightness(Image& img) : Filter(img) {};
    string getName() { return "Brightness"; };
    static string getId() { return "7"; };

    void apply() override {
        cout << "here is: " << value << '\n';
        for (int i = 0; i < image.width; i++) {
            for (int j = 0; j < image.height; j++) {
                for (int k = 0; k < 3; k++) {
                    int newValue = image(i, j, k) * value;
                    if (newValue > 255) newValue = 255;
                    if (newValue < 0) newValue = 0;
                    image(i, j, k) = newValue;
                }
            }
        }
    }

    vector<FilterParam> getNeeds() {
        return {
            {"Brightness (0:5)", "float", "1.0",0.0, 3.0}
        };
    }
    void setParam(const std::string& name, double value) {
        if (name == "Brightness (0:5)") this->value = value;
    }
};
class Crop : public Filter {
    int corner[2]{ 0 };
    int dimensions[2]{ 100 };

public:

    vector<FilterParam> getNeeds() override {
        return {
            {"X Corner", "int", "0", 0, double(image.width-1)},
            {"Y Corner", "int", "0", 0, double(image.height-1)},
            {"Width", "int", "100", 1, double(image.width)},
            {"Height", "int", "100", 1, double(image.height)}
        };
    }

    // 🛠️ setter مشابه لـ Resize
    void setParam(const std::string& name, double value) override {
        if (name == "X Corner") corner[0] = (int)value;
        else if (name == "Y Corner") corner[1] = (int)value;
        else if (name == "Width") dimensions[0] = (int)value;
        else if (name == "Height") dimensions[1] = (int)value;
    }

    // 🧩 لو محتاج تعيين مباشر (من الكروب بالماوس)
    void setCropParams(int x, int y, int w, int h) {
        corner[0] = x; corner[1] = y;
        dimensions[0] = w; dimensions[1] = h;
    }

    Crop(Image& img) :Filter(img) {};
    string getName() { return "Crop"; };
    static string getId() { return "8"; };

    void apply() override {
        Image croppedImage(dimensions[0], dimensions[1]);

        for (int i = corner[0], I = 0; i < (corner[0] + dimensions[0]); i++, I++) {
            for (int j = corner[1], J = 0; j < (corner[1] + dimensions[1]); j++, J++) {
                for (int k = 0; k < image.channels; k++) {
                    croppedImage(I, J, k) = image(i, j, k);
                }
            }
        }


        image = croppedImage;
    }
};
class Resize : public Filter {
    int dimensions[2]{ 100 };
    bool keepAspect = true;

public:
    Resize(Image& img) :Filter(img) {};
    vector<FilterParam> getNeeds() {
        return {
            {"Width", "int", "800", 10, double(image.width)},
            {"Height", "int", "600", 10, double(image.height)},
            {"Keep Aspect Ratio", "bool", "1", 0, 1}
        };
    }
    void setParam(const std::string& name, double value) {
        if (name == "Width") dimensions[0] = (int)value;
        else if (name == "Height") dimensions[1] = (int)value;
        else if (name == "Keep Aspect Ratio") keepAspect = (bool)value;
    }
    string getName() { return "Resizing"; };
    static string getId() { return "11"; };

    void apply() override {
        resizeImage(image, dimensions[0], dimensions[1]);
    }
};
class OilPainting : public Filter {
protected:
    int intensityLevels;

public:
    OilPainting(Image& img) :Filter(img) {};
    vector<FilterParam> getNeeds() {
        return {
            {"Detail Level (10:70)", "int", "20",10, 30}
        };
    }
    void setParam(const std::string& name, double value) {
        if (name == "Detail Level (10:70)") intensityLevels = (int)value;
    }

    string getName() { return "Oil Painting"; };
    static string getId() { return "14"; };

    void apply() override {
        Image output(image.width, image.height);
        for (int x = 0; x < image.width; x++) {
            for (int y = 0; y < image.height; y++) {

                for (int k = 0; k < image.channels; k++) {
                    int intensity = image(x, y, k);
                    int binIndex = (intensity * intensityLevels) / 255;
                    if (binIndex >= intensityLevels) binIndex = intensityLevels - 1;

                    int newIntensity = (binIndex * 255) / (intensityLevels - 1);

                    output(x, y, k) = newIntensity;
                }
            }
        }
        image = output;
    }
};
class ArtisticBrush : public OilPainting {
    int radius;

public:
    ArtisticBrush(Image& img) : OilPainting(img) {};
    vector<FilterParam> getNeeds() {
        return {
            {"Brush Width (2:7)", "int", "3", 2, 7},
            {"Detail Level (10:70)", "int", "20",10, 30}
        };
    }
    void setParam(const std::string& name, double value) {
        if (name == "Brush Width (2:7)") radius = (int)value;
        else if (name == "Detail Level (10:70)") intensityLevels = (int)value;
    }
    string getName() { return "Artistic Brush"; };
    static string getId() { return "22"; };

    void apply() override {
        Image output(image.width, image.height);

        for (int x = 0; x < image.width; x++) {
            for (int y = 0; y < image.height; y++) {
                vector<int> intensityCount(intensityLevels, 0);
                vector<int> avgR(intensityLevels, 0);
                vector<int> avgG(intensityLevels, 0);
                vector<int> avgB(intensityLevels, 0);

                int count = 0;
                for (int ny = y - radius; ny <= (y + radius); ny++) {
                    for (int nx = x - radius; nx <= (x + radius); nx++) {
                        if (isInBound(nx, ny)) {
                            count++;
                            int r = image(nx, ny, 0);
                            int g = image(nx, ny, 1);
                            int b = image(nx, ny, 2);

                            int intensity = (r + g + b) / 3;

                            int binIndex = (intensity * (intensityLevels - 1)) / 255;

                            intensityCount[binIndex]++;
                            avgR[binIndex] += r;
                            avgG[binIndex] += g;
                            avgB[binIndex] += b;
                        }
                    }
                }

                int maxBin = 0;
                int maxCount = 0;
                for (int i = 0; i < intensityLevels; i++) {
                    if (maxCount < intensityCount[i]) {
                        maxBin = i;
                        maxCount = intensityCount[i];
                    }
                }

                if (maxCount) { // I mean maxCount > 0; but logically if maxCount was not zero then it is a true value;
                    output(x, y, 0) = avgR[maxBin] / maxCount;
                    output(x, y, 1) = avgG[maxBin] / maxCount;
                    output(x, y, 2) = avgB[maxBin] / maxCount;
                }
            }
        }
        image = output;
    }

};
class Infrared : public Filter {
    int radius;

public:
    Infrared(Image& img) : Filter(img) {};
    vector<FilterParam> getNeeds() override {return {};};
    string getName() { return "Infrared"; };
    static string getId() { return "17"; };

    void apply() override {
        for (int x = 0; x < image.width; x++) {
            for (int y = 0; y < image.height; y++) {
                int redChannel = 0;
                for (int k = 0; k < image.channels; k++) {
                    redChannel += image(x, y, k);
                }
                redChannel /= image.channels;

                image(x, y, 0) = 255;
                image(x, y, 1) = 255 - redChannel;
                image(x, y, 2) = 255 - redChannel;
            }
        }
    }
};
class Bloody : public Filter {
    int radius;

public:
    Bloody(Image& img) : Filter(img) {};
    vector<FilterParam> getNeeds() override {return {};};
    string getName() { return "Bloody"; };
    static string getId() { return "19"; };

    void apply() override {
        for (int x = 0; x < image.width; x++) {
            for (int y = 0; y < image.height; y++) {
                int redChannel = 0;
                for (int k = 0; k < image.channels; k++) {
                    redChannel += image(x, y, k);
                }
                redChannel /= image.channels;

                image(x, y, 0) = redChannel;
                image(x, y, 1) = 0;
                image(x, y, 2) = 0;
            }
        }
    }

};
class Sky : public Filter {
public:
    Sky(Image& img) : Filter(img) {};
    vector<FilterParam> getNeeds() override {return {};};
    string getName() { return "Sky"; };
    static string getId() { return "21"; };

    void apply() override {
        for (int x = 0; x < image.width; x++) {
            for (int y = 0; y < image.height; y++) {
                int blueChannel = 0;
                for (int k = 0; k < image.channels; k++) {
                    blueChannel += image(x, y, k);
                }
                blueChannel /= image.channels;

                image(x, y, 0) = 0;
                image(x, y, 1) = blueChannel / 2;
                image(x, y, 2) = blueChannel;
            }
        }
    }

};
class Grass: public Filter {

public:
    Grass(Image& img) : Filter(img) {};
    string getName() { return "Grass"; };
    static string getId() { return "20"; };

    void apply() override {
        for (int x = 0; x < image.width; x++) {
            for (int y = 0; y < image.height; y++) {
                int intensity = 0;
                for (int k = 0; k < image.channels; k++) {
                    intensity += image(x, y, k);
                }
                intensity /= image.channels;

                image(x, y, 0) = 0;
                image(x, y, 1) = intensity;
                image(x, y, 2) = 0;
            }
        }

    }
    vector<FilterParam> getNeeds() override {return {};};

};
class Frame : public Filter {
    int Thickness;
    int R, G, B;
    bool isDecorative;

public:
    Frame(Image& img) : Filter(img), R(0), G(0), B(0), Thickness(1), isDecorative(false) {}

    string getName() { return "Frame"; }
    static string getId() { return "9"; }

    vector<FilterParam> getNeeds() {
        return {
            {"Frame Type (1=Normal, 2=Decorative)", "int","1", 1, 2},
            {"Frame Color", "color","red", 0, 0},
            {"Thickness", "int","10", 1, 50}
        };
    }

    void setParam(const std::string& name, double value) {
        if (name.find("Frame Type") != string::npos) isDecorative = (value == 2);
        if (name == "Thickness") Thickness = value;
    }

    void setParam(const std::string& name, const std::string& color) {
        if (color.size() == 7 && color[0] == '#') {
            R = std::stoi(color.substr(1, 2), nullptr, 16);
            G = std::stoi(color.substr(3, 2), nullptr, 16);
            B = std::stoi(color.substr(5, 2), nullptr, 16);
        } else {
            R = G = B = 0; // fallback لو الصيغة غلط
        }
    }

    void apply() override
    {
        int width = image.width;
        int height = image.height;

        if (!isDecorative) {
            for (int i = 0; i < width; i++) {
                for (int t = 0; t < Thickness; t++) {
                    image(i, t, 0) = R;
                    image(i, t, 1) = G;
                    image(i, t, 2) = B;
                }
            }
            for (int i = 0; i < width; i++) {
                for (int t = 0; t < Thickness; t++) {
                    image(i, height - 1 - t, 0) = R;
                    image(i, height - 1 - t, 1) = G;
                    image(i, height - 1 - t, 2) = B;
                }
            }
            for (int i = 0; i < height; i++) {
                for (int t = 0; t < Thickness; t++) {
                    image(t, i, 0) = R;
                    image(t, i, 1) = G;
                    image(t, i, 2) = B;
                }
            }
            for (int i = 0; i < height; i++) {
                for (int t = 0; t < Thickness; t++) {
                    image(width - 1 - t, i, 0) = R;
                    image(width - 1 - t, i, 1) = G;
                    image(width - 1 - t, i, 2) = B;
                }
            }
        }

        else
        {
            RGB inner = { 255, 255, 255 };
            int outerThickness = Thickness;
            int innerThickness = Thickness / 2;
            for (int i = 0; i < width; i++) {
                for (int t = 0; t < outerThickness; t++) {
                    image(i, t, 0) = R;
                    image(i, t, 1) = G;
                    image(i, t, 2) = B;
                }
            }
            for (int i = 0; i < width; i++)
            {
                for (int t = 0; t < outerThickness; t++)
                {
                    image(i, height - 1 - t, 0) = R;
                    image(i, height - 1 - t, 1) = G;
                    image(i, height - 1 - t, 2) = B;
                }
            }
            for (int i = 0; i < height; i++)
            {
                for (int t = 0; t < outerThickness; t++) {
                    image(t, i, 0) = R;
                    image(t, i, 1) = G;
                    image(t, i, 2) = B;
                }
            }
            for (int i = 0; i < height; i++)
            {
                for (int t = 0; t < outerThickness; t++) {
                    image(width - 1 - t, i, 0) = R;
                    image(width - 1 - t, i, 1) = G;
                    image(width - 1 - t, i, 2) = B;
                }
            }
            for (int i = outerThickness; i < width - outerThickness; i++)
            {
                for (int t = 0; t < innerThickness; t++) {
                    image(i, outerThickness + t, 0) = inner.R;
                    image(i, outerThickness + t, 1) = inner.G;
                    image(i, outerThickness + t, 2) = inner.B;
                }
            }
            for (int i = outerThickness; i < width - outerThickness; i++)
            {
                for (int t = 0; t < innerThickness; t++) {
                    image(i, height - outerThickness - 1 - t, 0) = inner.R;
                    image(i, height - outerThickness - 1 - t, 1) = inner.G;
                    image(i, height - outerThickness - 1 - t, 2) = inner.B;
                }
            }
            for (int i = outerThickness; i < height - outerThickness; i++)
            {
                for (int t = 0; t < innerThickness; t++)
                {
                    image(outerThickness + t, i, 0) = inner.R;
                    image(outerThickness + t, i, 1) = inner.G;
                    image(outerThickness + t, i, 2) = inner.B;
                }
            }
            for (int i = outerThickness; i < height - outerThickness; i++)
            {
                for (int t = 0; t < innerThickness; t++)
                {
                    image(width - outerThickness - 1 - t, i, 0) = inner.R;
                    image(width - outerThickness - 1 - t, i, 1) = inner.G;
                    image(width - outerThickness - 1 - t, i, 2) = inner.B;
                }
            }
        }


    }
};
class EdgeDetection : public Filter {

public:
    EdgeDetection(Image& img) : Filter(img){}

    string getName() { return "Edge Detection"; }
    static string getId() { return "10"; }
    static map<string,vector<vector<int>>> sobelKernels () {
        return {
            {"Gx",{
                       {-1,0,1},
                       {-2,0,2},
                       {-1,0,1}
                   }},
            {"Gy", {
                       {-1,-2,-1},
                       {0,0,0},
                       {1,2,1}
                   }}
        };
    }
    vector<FilterParam> getNeeds() override {return {};};
    void apply() override {
        GreyScale grey(image);
        grey.apply();
        Blur blur(image, 2);
        blur.apply();

        Image output(image.width, image.height);
        map<string,vector<vector<int>>> kernels = sobelKernels();
        computeThreshold();
        for (int x = 1; x < image.width-1; x++) {
            for (int y = 1; y < image.height-1; y++) {
                int sumX = 0;
                int sumY = 0;
                for (int i = -1; i <= 1; i++ ) {
                    for (int j = -1; j <= 1; j++ ) {
                        int intensity = image(x+i,y+j,0);
                        sumX += intensity * kernels["Gx"][i+1][j+1];
                        sumY += intensity * kernels["Gy"][i+1][j+1];
                    }
                }
                int magnitude = sqrt((sumX*sumX)+(sumY*sumY));
                if (magnitude > threshold) magnitude = 0;
                else magnitude = 255;
                output(x,y,0) = output(x,y,1) = output(x,y,2) = magnitude;
            }
        }

        image = output;
    }
};


class Gama : public Filter
{
    double gama;
public:
    Gama(Image& img) : Filter(img) {};
    string getName() { return "Gama"; };
    static string getId(){ return "25"; };
    void apply() override {
        for (int i = 0; i < image.width; i++)
        {
            for (int j = 0; j < image.height;j++)
            {
                float Val = pow(image(i, j, 0) / 255.0f, gama);
                int Nr = min(int(255 * Val), 255);
                image(i, j, 0) = Nr;
                Val = pow(image(i, j, 1) / 255.0f, gama);
                int Ng = min(int(255 * Val), 255);
                image(i, j, 1) = Ng;

                Val = pow(image(i, j, 2) / 255.0f, gama);
                int Nb = min(int(255 * Val), 255);
                image(i, j, 2) = Nb;
            }
        }
    };

    vector<FilterParam> getNeeds() {
        return {
            {"Enter Value between [0 , 10]", "float", "1.0",0.0, 10}
        };
    }
    void setParam(const std::string& name, double value) {
        if (name == "Enter Value between [0 , 10]") this->gama = value;
    }
};


class HeatMap : public Filter
{
    double p;
public:
    HeatMap(Image& img) : Filter(img) {};
    string getName() { return "Heat Map"; };
    static string getId(){ return "26"; };
    void apply() override {
        for (int i = 0; i < image.width; i++){
            for (int j = 0;j < image.height;j++)
            {
                float R = static_cast<float>(image(i, j, 0));
                float G = static_cast<float>(image(i, j, 1));
                float B = static_cast<float>(image(i, j, 2));
                float mean_ntensity = (R + G + B) / 3.0f;

                if (mean_ntensity < 64)
                {
                    image(i , j , 0 ) = 0 ;
                    image(i, j, 1) = 0;
                    image(i, j, 2) = 255;
                }
                else if (mean_ntensity < 128)
                {
                    image(i, j, 0) = 0;
                    image(i, j, 1) = 255;
                    image(i, j, 2) =0;
                }
                else if (mean_ntensity < 192)
                {
                    image(i, j, 0) = 255;
                    image(i, j, 1) = 255;
                    image(i, j, 2) = 0;
                }
                else
                {
                    image(i, j, 0) = 255;
                    image(i, j, 1) = 0;
                    image(i, j, 2) = 0;
                }

            }
        }
    };

    vector<FilterParam> getNeeds() {
        return {
        };
    }
};



class Saturation : public Filter
{
    double p;
public:
    Saturation(Image& img) : Filter(img) {};
    string getName() { return "Saturation"; };
    static string getId(){ return "23"; };
    HSV rgbToHsv(const RGB& rgb) {
        float r = rgb.R / 255.0f;
        float g = rgb.G / 255.0f;
        float b = rgb.B / 255.0f;

        float maxVal = max({ r, g, b });
        float minVal = min({ r, g, b });
        float diff = maxVal - minVal;

        HSV hsv;
        hsv.v = maxVal;

        if (maxVal == 0)
            hsv.s = 0;
        else
            hsv.s = diff / maxVal;

        if (diff == 0)
            hsv.h = 0;
        else if (maxVal == r)
            hsv.h = 60 * fmod(((g - b) / diff), 6.0);
        else if (maxVal == g)
            hsv.h = 60 * (((b - r) / diff) + 2);
        else
            hsv.h = 60 * (((r - g) / diff) + 4);

        if (hsv.h < 0) hsv.h += 360;

        return hsv;
    }
    RGB hsvToRgb(const HSV& hsv) {
        double C = hsv.v * hsv.s;
        double X = C * (1 - fabs(fmod(hsv.h / 60.0, 2) - 1));
        double m = hsv.v - C;

        double rPrime, gPrime, bPrime;

        if (hsv.h >= 0 && hsv.h < 60)
            rPrime = C, gPrime = X, bPrime = 0;
        else if (hsv.h >= 60 && hsv.h < 120)
            rPrime = X, gPrime = C, bPrime = 0;
        else if (hsv.h >= 120 && hsv.h < 180)
            rPrime = 0, gPrime = C, bPrime = X;
        else if (hsv.h >= 180 && hsv.h < 240)
            rPrime = 0, gPrime = X, bPrime = C;
        else if (hsv.h >= 240 && hsv.h < 300)
            rPrime = X, gPrime = 0, bPrime = C;
        else
            rPrime = C, gPrime = 0, bPrime = X;

        RGB out;
        out.R = (rPrime + m) * 255;
        out.G = (gPrime + m) * 255;
        out.B = (bPrime + m) * 255;

        return out;
    }


    void apply() override {
        for (int i = 0; i < image.width; i++) {
            for (int j = 0; j < image.height; j++) {
                RGB color = { image(i, j, 0), image(i, j, 1), image(i, j, 2) };
                HSV hsv = rgbToHsv(color);

                hsv.s *= (p / 100.0f);
                hsv.s = min(max(hsv.s, 0.0f), 1.0f);

                RGB newColor = hsvToRgb(hsv);

                image(i, j, 0) = static_cast<unsigned char>(newColor.R);
                image(i, j, 1) = static_cast<unsigned char>(newColor.G);
                image(i, j, 2) = static_cast<unsigned char>(newColor.B);
            }
        }
    };

    vector<FilterParam> getNeeds() {
        return {
            {"Enter saturation percentage (100 = normal, >100 = more color, <100 = less color):", "float", "1.0",0.0, 10}
        };
    }
    void setParam(const std::string& name, double value) {
        if (name == "Enter saturation percentage (100 = normal, >100 = more color, <100 = less color):") this->p = value;
    }
};



class OldPhoto : public Filter
{
    double p;
public:
    OldPhoto (Image& img) : Filter(img) {};
    string getName() { return "Old Photo"; };
    static string getId(){ return "24"; };

    void apply() override {
        for (int i = 0; i < image.width; i++)
        {
            for (int j = 0; j < image.height; j++)
            {

                int Nr = 0.4 * image(i, j, 0 ) + 0.75 * image(i, j, 1) + 0.2 * image(i, j, 2);
                Nr = min(255, Nr);
                int Ng = 0.35* image(i, j, 0)  + 0.7 * image(i, j, 1) + 0.15*image(i, j, 2);
                Ng = min(255, Ng);
                int Nb = 0.2 * image(i, j, 0) +  0.55*image(i, j, 1)  + 0.13* image(i, j, 2);
                Nb = min(255, Nb);
                image(i, j, 0) = Nr;
                image(i, j, 1) = Ng;
                image(i, j, 2) = Nb;
            }
        }
    }

    vector<FilterParam> getNeeds() {
        return {
        };
    }
};


#include <cstdlib>
#include <ctime>
#include <algorithm>

class Snow : public Filter
{
    double p;
public:
    Snow(Image& img) : Filter(img) {
        srand(time(0));
    };
    string getName() { return "Snow"; };
    static string getId(){ return "27"; };

    void apply() override {
        int numSnow = (image.width * image.height) / 30;
        for (int i = 0; i < numSnow; i++) {
            int x = rand() % image.width;
            int y = rand() % image.height;

            int snowValue = 200 + rand() % 56;

            image(x, y, 0) = snowValue;
            image(x, y, 1) = snowValue;
            image(x, y, 2) = snowValue;

        }
    }

    vector<FilterParam> getNeeds() {
        return {
        };
    }
};




