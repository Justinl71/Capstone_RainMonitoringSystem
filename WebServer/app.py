from flask import Flask, render_template, url_for, request, redirect, session, Response, json
from flask_sqlalchemy import SQLAlchemy
from datetime import datetime
import pandas as pd
import os
from sqlalchemy import func
import io
from matplotlib.backends.backend_agg import FigureCanvasAgg as FigureCanvas
from matplotlib.figure import Figure
from flask_googlemaps import GoogleMaps

app = Flask(__name__)
app.config['SQLALCHEMY_DATABASE_URI'] = 'sqlite:///test.db'
db = SQLAlchemy(app)

class Todo(db.Model):
    EntryID = db.Column(db.Integer, primary_key=True)
    NodeID = db.Column(db.String(200), nullable=False)
    RainData = db.Column(db.Integer)
    Date = db.Column(db.DateTime, default=datetime.utcnow)

    def __repr__(self):
        return '<Task %r>' % self.EntryID
        
class locations(db.Model):
    NodeID = db.Column(db.Integer, primary_key=True)
    Long = db.Column(db.Float)
    Lat = db.Column(db.Float)
        
def create_figure(Noders):
	plotThis = Todo.query.order_by(Todo.Date.desc()).all()
	fig = Figure()
	axis = fig.add_subplot(1, 1, 1)
	xs = range(100)
	ys = [0] * 100
	counter = 0
	for items in plotThis:
		if counter >= 99:
			break
		if items.NodeID == Noders:
			ys[counter] = items.RainData;
			counter += 1
	axis.set_title(Noders)
	axis.set_xlabel('Day Number (Past 100 days)')
	axis.set_ylabel('Rain(mm)')
	axis.bar(xs, ys)
	return fig

@app.route('/', methods=['POST', 'GET'])
def index():
	if request.method == 'POST':
		return redirect('/')
	else:
		tasks = Todo.query.order_by(Todo.Date.desc()).all()
		return render_template('index.html', tasks=tasks)
        	
@app.route('/delete/<int:EntryID>')
def delete(EntryID):
	task_to_delete = Todo.query.get_or_404(EntryID)
	try:
		db.session.delete(task_to_delete)
		db.session.commit()
		return redirect('/')
	except:
		return 'There was a problem deleting that task'
		
@app.route('/upload', methods=['POST'])
def upload_file():
	uploaded_file = request.files['file']
	if uploaded_file.filename != '':
		uploaded_file.save(uploaded_file.filename)
		df = pd.read_csv ('DATALOG.csv')
		for index, rows in df.iterrows():
			final_string = rows.Date + " " + rows.Time
			date_time_obj = datetime.strptime(final_string, '%Y-%m-%d %H:%M:%S')
			new_task = Todo(NodeID=rows['NodeID'], RainData=rows['RainValue'], Date=date_time_obj)
			db.session.add(new_task)
			db.session.commit()
		os.remove("DATALOG.csv")
	return redirect(url_for('index'))
	
@app.route('/plot1.png')
def plot_png1():
	fig = create_figure("A081")
	output = io.BytesIO()
	FigureCanvas(fig).print_png(output)
	return Response(output.getvalue(), mimetype='image/png')
	
@app.route('/plot2.png')
def plot_png2():
	fig = create_figure("A082")
	output = io.BytesIO()
	FigureCanvas(fig).print_png(output)
	return Response(output.getvalue(), mimetype='image/png')

@app.route('/plot3.png')
def plot_png3():
	fig = create_figure("A083")
	output = io.BytesIO()
	FigureCanvas(fig).print_png(output)
	return Response(output.getvalue(), mimetype='image/png')
	
@app.route('/locations', methods=['POST', 'GET'])
def display_map():

	if request.method == 'POST':
		task = locations.query.get_or_404(request.form['nodeName'])
		task.Long = request.form['Lng']
		task.Lat = request.form['Lat']
		#new_node = locations(NodeID=request.form['nodeName'], Long=request.form['Lng'], Lat=request.form['Lat'])
		#db.session.add(new_node)
		db.session.commit()
		return redirect('/')
	else:
		nodes = locations.query.order_by(locations.NodeID).all()
		count = 0
		for noder in nodes:
			count += 1
		return render_template('mapa.html', nodes=nodes, count=count)

if __name__ == "__main__":
	app.run(debug=True)
