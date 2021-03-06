#ifndef DbTables_TrkDelayPanel_hh
#define DbTables_TrkDelayPanel_hh


#include <string>
#include <iomanip>
#include <sstream>
#include <map>
#include "DbTables/inc/DbTable.hh"

namespace mu2e {

  class TrkDelayPanel : public DbTable {
  public:

    typedef std::shared_ptr<TrkDelayPanel> ptr_t;
    typedef std::shared_ptr<const TrkDelayPanel> cptr_t;

    class Row {
    public:
      Row(int index, float delay):_index(index),_delay(delay) {}
      int  index() const { return _index;}
      float delay() const {return _delay;}
    private:
      int _index;
      float _delay;
    };

    constexpr static const char* cxname = "TrkDelayPanel";

    TrkDelayPanel():DbTable(cxname,"trk.delaypanel",
	 "index,delay") {}
    const Row& rowAt(const std::size_t index) const { return _rows.at(index);}
    std::vector<Row> const& rows() const {return _rows;}
    std::size_t nrow() const override { return _rows.size(); };
    virtual std::size_t nrowFix() const override { return 216; }; 
    size_t size() const override { return baseSize() + nrow()*sizeof(Row); };

    void addRow(const std::vector<std::string>& columns) override {
      _rows.emplace_back(std::stoi(columns[0]),
			 std::stof(columns[1]) );
    }

    void rowToCsv(std::ostringstream& sstream, std::size_t irow) const override {
      Row const& r = _rows.at(irow);
      sstream << r.index()<<",";
      sstream << std::fixed << std::setprecision(1);
      sstream << r.delay();
    }

    virtual void clear() override { baseClear(); _rows.clear(); }

  private:
    std::vector<Row> _rows;
  };
  
};
#endif
