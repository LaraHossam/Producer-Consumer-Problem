.PHONY: clean

producer:   	
	rm -f producer
	g++ -o producer producer.cc
consumer:   	
	rm -f consumer
	g++ -o consumer consumer.cc

clean:
	rm -f producer
	rm -f consumer

