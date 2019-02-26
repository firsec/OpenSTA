// OpenSTA, Static Timing Analyzer
// Copyright (c) 2019, Parallax Software, Inc.
// 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#ifndef STA_TIMING_ARC_H
#define STA_TIMING_ARC_H

#include "DisallowCopyAssign.hh"
#include "Vector.hh"
#include "Transition.hh"
#include "LibertyClass.hh"

namespace sta {

class TimingArcAttrs;
class WireTimingArc;
class WireTimingArcSetArcIterator;
class TimingArcSetArcIterator;

typedef int TimingArcIndex;
typedef Vector<TimingArc*> TimingArcSeq;
typedef Map<const OperatingConditions*, TimingModel*> ScaledTimingModelMap;

typedef enum {
  timing_type_clear,
  timing_type_combinational,
  timing_type_combinational_fall,
  timing_type_combinational_rise,
  timing_type_falling_edge,
  timing_type_hold_falling,
  timing_type_hold_rising,
  timing_type_min_pulse_width,
  timing_type_minimum_period,
  timing_type_nochange_high_high,
  timing_type_nochange_high_low,
  timing_type_nochange_low_high,
  timing_type_nochange_low_low,
  timing_type_non_seq_hold_falling,
  timing_type_non_seq_hold_rising,
  timing_type_non_seq_setup_falling,
  timing_type_non_seq_setup_rising,
  timing_type_preset,
  timing_type_recovery_falling,
  timing_type_recovery_rising,
  timing_type_removal_falling,
  timing_type_removal_rising,
  timing_type_retaining_time,
  timing_type_rising_edge,
  timing_type_setup_falling,
  timing_type_setup_rising,
  timing_type_skew_falling,
  timing_type_skew_rising,
  timing_type_three_state_disable,
  timing_type_three_state_disable_fall,
  timing_type_three_state_disable_rise,
  timing_type_three_state_enable,
  timing_type_three_state_enable_fall,
  timing_type_three_state_enable_rise,
  timing_type_min_clock_tree_path,
  timing_type_max_clock_tree_path,
  timing_type_unknown
} TimingType;

void
makeTimingTypeMap();
void
deleteTimingTypeMap();

const char *
timingTypeString(TimingType type);
TimingType
findTimingType(const char *string);
bool
timingTypeIsCheck(TimingType type);
ScaleFactorType
timingTypeScaleFactorType(TimingType type);

////////////////////////////////////////////////////////////////

class TimingArcAttrs
{
public:
  TimingArcAttrs();
  virtual ~TimingArcAttrs();
  void deleteContents();
  TimingType timingType() const { return timing_type_; }
  void setTimingType(TimingType type);
  TimingSense timingSense() const { return timing_sense_; }
  void setTimingSense(TimingSense sense);
  FuncExpr *cond() const { return cond_; }
  FuncExpr *&condRef() { return cond_; }
  const char *sdfCond() const { return sdf_cond_; }
  void setSdfCond(const char *cond);
  const char *sdfCondStart() const { return sdf_cond_start_; }
  void setSdfCondStart(const char *cond);
  const char *sdfCondEnd() const { return sdf_cond_end_; }
  void setSdfCondEnd(const char *cond);
  const char *modeName() const { return mode_name_; }
  void setModeName(const char *name);
  const char *modeValue() const { return mode_value_; }
  void setModeValue(const char *value);
  TimingModel *model(TransRiseFall *tr) const;
  void setModel(TransRiseFall *tr,
		TimingModel *model);
  float ocvArcDepth() const { return ocv_arc_depth_; }
  void setOcvArcDepth(float depth);

protected:
  TimingType timing_type_;
  TimingSense timing_sense_;
  FuncExpr *cond_;
  const char *sdf_cond_;
  const char *sdf_cond_start_;
  const char *sdf_cond_end_;
  const char *mode_name_;
  const char *mode_value_;
  float ocv_arc_depth_;
  TimingModel *models_[TransRiseFall::index_count];

private:
  DISALLOW_COPY_AND_ASSIGN(TimingArcAttrs);
};

// A timing arc set is a group of related timing arcs between from/to
// a pair of cell ports.  Wire timing arcs are a special set owned by
// the TimingArcSet class.
//
// See ~LibertyCell for delete of TimingArcSet members.
class TimingArcSet
{
public:
  TimingArcSet(LibertyCell *cell,
	       LibertyPort *from,
	       LibertyPort *to,
	       LibertyPort *related_out,
	       TimingRole *role,
	       TimingArcAttrs *attrs);
  virtual ~TimingArcSet();
  LibertyCell *libertyCell() const;
  LibertyPort *from() const { return from_; }
  LibertyPort *to() const { return to_; }
  LibertyPort *relatedOut() const { return related_out_; }
  TimingRole *role() const { return role_; };
  TimingSense sense() const;
  // Rise/fall if the arc set is rising_edge or falling_edge.
  TransRiseFall *isRisingFallingEdge() const;
  size_t arcCount() const { return arcs_.size(); }
  TimingArcSeq &arcs() { return arcs_; }
  // Return 1 or 2 arcs matching from transition.
  void arcsFrom(const TransRiseFall *from_tr,
		// Return values.
		TimingArc *&arc1,
		TimingArc *&arc2);
  const TimingArcSeq &arcs() const { return arcs_; }
  // Use the TimingArcSetArcIterator(arc_set) constructor instead.
  TimingArcSetArcIterator *timingArcIterator();
  TimingArcIndex addTimingArc(TimingArc *arc);
  void deleteTimingArc(TimingArc *arc);
  TimingArc *findTimingArc(unsigned arc_index);
  void setRole(TimingRole *role);
  FuncExpr *cond() const { return cond_; }
  // Cond default is the timing arcs with no condition when there are
  // other conditional timing arcs between the same pins.
  bool isCondDefault() const { return is_cond_default_; }
  void setIsCondDefault(bool is_default);
  // SDF IOPATHs match sdfCond.
  // sdfCond (IOPATH) reuses sdfCondStart (timing check) variable.
  const char *sdfCond() const { return sdf_cond_start_; }
  // SDF timing checks match sdfCondStart/sdfCondEnd.
  const char *sdfCondStart() const { return sdf_cond_start_; }
  const char *sdfCondEnd() const { return sdf_cond_end_; }
  const char *modeName() const { return mode_name_; }
  const char *modeValue() const { return mode_value_; }
  // Timing arc set index in cell.
  TimingArcIndex index() const { return index_; }
  bool isDisabledConstraint() const { return is_disabled_constraint_; }
  void setIsDisabledConstraint(bool is_disabled);
  // OCV arc depth from timing/cell/library.
  float ocvArcDepth() const;

  static bool equiv(const TimingArcSet *set1,
		    const TimingArcSet *set2);
  static bool less(const TimingArcSet *set1,
		   const TimingArcSet *set2);

  static void init();
  static void destroy();

  // Psuedo definition for wire arcs.
  static TimingArcSet *wireTimingArcSet() { return wire_timing_arc_set_; }
  static int wireArcIndex(const TransRiseFall *tr);
  static int wireArcCount() { return 2; }

protected:
  void init(LibertyCell *cell);
  TimingArcSet(TimingRole *role);

  LibertyPort *from_;
  LibertyPort *to_;
  LibertyPort *related_out_;
  TimingRole *role_;
  TimingArcSeq arcs_;
  FuncExpr *cond_;
  bool is_cond_default_;
  const char *sdf_cond_start_;
  const char *sdf_cond_end_;
  const char *mode_name_;
  const char *mode_value_;
  float ocv_arc_depth_;
  unsigned index_;
  bool is_disabled_constraint_;
  TimingArc *from_arc1_[TransRiseFall::index_count];
  TimingArc *from_arc2_[TransRiseFall::index_count];

  static TimingArcSet *wire_timing_arc_set_;

private:
  DISALLOW_COPY_AND_ASSIGN(TimingArcSet);
};

class TimingArcSetArcIterator : public TimingArcSeq::ConstIterator
{
public:
  TimingArcSetArcIterator(const TimingArcSet *set);

private:
  DISALLOW_COPY_AND_ASSIGN(TimingArcSetArcIterator);
};

// A timing arc is a single from/to transition between two ports.
// The timing model parameters used for delay calculation are also found here.
class TimingArc
{
public:
  TimingArc(TimingArcSet *set,
	    Transition *from_tr,
	    Transition *to_tr,
	    TimingModel *model);
  ~TimingArc();
  LibertyPort *from() const { return set_->from(); }
  LibertyPort *to() const { return set_->to(); }
  Transition *fromTrans() const { return from_tr_; }
  Transition *toTrans() const { return to_tr_; }
  TimingRole *role() const { return set_->role(); }
  TimingArcSet *set() const { return set_; }
  TimingSense sense() const;
  // Index in TimingArcSet.
  unsigned index() const { return index_; }
  TimingModel *model(const OperatingConditions *op_cond) const;
  TimingModel *model() const { return model_; }
  TimingArc *cornerArc(int ap_index);
  void setCornerArc(TimingArc *corner_arc,
		    int ap_index);

  static bool equiv(const TimingArc *arc1,
		    const TimingArc *arc2);

protected:
  void setIndex(unsigned index);
  void addScaledModel(const OperatingConditions *op_cond,
		      TimingModel *scaled_model);

  TimingArcSet *set_;
  Transition *from_tr_;
  Transition *to_tr_;
  unsigned index_;
  TimingModel *model_;
  ScaledTimingModelMap *scaled_models_;
  Vector<TimingArc*> corner_arcs_;

private:
  DISALLOW_COPY_AND_ASSIGN(TimingArc);

  friend class LibertyLibrary;
  friend class LibertyCell;
  friend class TimingArcSet;
};

} // namespace
#endif
